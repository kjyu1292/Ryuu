section .note.GNU-stack

recurDeg_d				equ	624
offSet_d				equ	397
genRange_d				equ	32-2
twistVal_d				equ	31

MASK					equ	0xffffffff
UMASK_d					equ	((MASK << twistVal_d) & MASK)
LMASK_d					equ	((MASK >> (genRange_d+2 - twistVal_d)) & MASK)
twistCoef_d				equ	(0x9908b0df & MASK)

tempU_d					equ	11
tempS_d					equ	7
tempT_d					equ	15
tempL_d					equ	18
tempB_d					equ	(0x9d2c5680 & MASK)
tempC_d					equ	(0xefc60000 & MASK)

initMul_d				equ	1812433253

section .rodata
	align 16

	mulMask				dq	0xFFFFFFFF00000000, 0xFFFFFFFF00000000

	recurDeg_d_arr			dd	recurDeg_d, recurDeg_d, recurDeg_d, recurDeg_d

	UMASK_d_arr			dd	UMASK_d, UMASK_d, UMASK_d, UMASK_d
	LMASK_d_arr			dd	LMASK_d, LMASK_d, LMASK_d, LMASK_d
	twistCoef_d_arr			dd	twistCoef_d, twistCoef_d, twistCoef_d, twistCoef_d

        tempB_d_arr			dd	tempB_d, tempB_d, tempB_d, tempB_d
        tempC_d_arr			dd	tempC_d, tempC_d, tempC_d, tempC_d

	initMul_d_arr			dd	initMul_d, initMul_d, initMul_d, initMul_d


;extern void initialize_state(unsigned int* starting_ptr, unsigned int* seed);
;extern unsigned int* random_uint32(unsigned int* starting_ptr, int* idx, int num_blocks);

section .text
	
	global		_initialize_state__m128
	global		_random_uint32__m128
	
	global		_initialize_state__m512
	global		_random_uint32__m512

_initialize_state__m128:
	push				rbp
	mov				rbp, rsp
	and				rsp, -16
	push				rbx
	
	xor				rbx, rbx
	xor				rcx, rcx
	add				rbx, 16
	mov				rcx, recurDeg_d
	sal				rcx, 4

	; move 4 uint32 seeds into xmm0
	vmovdqa				xmm0, [rdx]

	; state_array[127:0] = 4 initial seeds
	vmovdqa				[rdi], xmm0

	; pre-shift the constant vector initMull_d_arr for later operations
	; xmm3(in double word) = 0 genRange_d genRange_d genRange_d
	vmovdqa				xmm2, [rel initMul_d_arr]
	vpsrldq				xmm3, xmm2, 4
	vmovdqa				xmm6, [rel mulMask]
	
_L0x01:
	; seed >> (genRange_d - 2)
	vpsrld				xmm4, xmm0, genRange_d

	; seed ^ (seed >> (genRange_d - 2))
	vpxor				xmm0, xmm0, xmm4

	; initMul_d * (seed ^ (seed >> (genRange_d)))
	; xmm4[127:64] = xmm0[95:64] * xmm1[95:64]
	; xmm4[63:0]   = xmm0[31:0]  * xmm1[31:0]
	vpmuldq				xmm4, xmm0, xmm2

	; xmm5(in double word) = 0 xmm0[127:96] xmm0[95:64] xmm0[63:32]
	vpsrldq				xmm5, xmm0, 4
	; multiplication will double the bits of the sources
	vpmuldq				xmm5, xmm5, xmm3
	; revert the 32-bit to its og location
	vpslldq				xmm5, xmm5, 4

	; Treat the bytes as they are in bits, so that vshufps can be used
	; vshufps does not care about the sources' type, tho the instruction
	; is usually used to shuffle single precisions
	; Move the bits xmm4[95:64] and xmm4[31:0] to xmm0[63:0]
	; Move the bits xmm5[95:64] and xmm5[31:0] to xmm0[127:64]
	; vshufps				xmm0, xmm4, xmm5, 0b00100010
	; *** Use vpblendvb instead, similar op/en, less overhead
	vpblendvb			xmm0, xmm4, xmm5, xmm6

	; plus i
	sar				rbx, 4
	vmovd				xmm4, ebx
	sal				rbx, 4
	vbroadcastss			xmm4, xmm4
	vpaddd				xmm0, xmm0, xmm4

	; save
	vmovdqa				[rdi+rbx], xmm0

	add				rbx, 16
	cmp				rbx, rcx
	jbe				_L0x01

	mov				dword [rsi], 0

_end_initialize_state__m128:
	pop				rbx
	mov				rsp, rbp
	pop				rbp
	ret

_random_uint32__m128:
	push				rbp
	mov				rbp, rsp
	and				rsp, -16
	push				rbx
	push				r12

	xor				rax, rax
	xor				rbx, rbx
	xor				r8, r8
	xor				r12, r12
	
	vmovdqa				xmm2,  [rel UMASK_d_arr]
	vmovdqa				xmm3,  [rel LMASK_d_arr]
	vmovdqa				xmm4,  [rel twistCoef_d_arr]
        vmovdqa				xmm11, [rel tempB_d_arr]
        vmovdqa				xmm12, [rel tempC_d_arr]

_L0x02:
	; get the state index, k
	mov				ebx, dword [rsi]

	; j
	mov				rax, rbx
	sub				rax, recurDeg_d
	inc				rax

	; (j < 0) j += recurDeg_d
	mov				r12, rax
	sar				r12, 63
	and				r12, recurDeg_d
	add				rax, r12

	; state_array[k]
	shl				rbx, 4
	vmovdqa				xmm0, [rdi+rbx]
	shr				rbx, 4
	; state_array[j]
	shl				rax, 4
	vmovdqa				xmm1, [rdi+rax]
	shr				rax, 4

	; state_arr[k] & UMASK_d
	vpand				xmm0, xmm0, xmm2
	; state_arr[j] & LMASK_d
	vpand				xmm1, xmm1, xmm3
	; x = (state_arr[k] & UMASK_d | state_arr[j] & LMASK_d)
	vpor				xmm0, xmm0, xmm1

	; xA = x >> 1, 2 copies
	vpsrld				xmm1, xmm0, 1
	vmovdqa				xmm6, xmm1
	; _mask = x & 0x00000001 -> only the LSB of x matters
	vpslld				xmm5, xmm0, 31
	vpsrad				xmm5, xmm5, 31

	; xA = _mask & xA
	; first copy zero out all states that do not pass the cond
	; but these states must be preserved, this is where
	; the second copy comes in
	vpand				xmm1, xmm1, xmm5
	; xA ^= twistCoef_d
	vpxor				xmm1, xmm1, xmm4

	; e.g. the mask: 0xFs 0x0s 0x0s 0xFs 
	; carry out xor on 3rd and 0th indexes
	; preserve 1st and 2nd indexes
	; This must be turned in the bit flag to use vpblendvb
	vpblendvb 			xmm1, xmm6, xmm1, xmm5

	; j = k - recurDeg_d + offSet_d
	mov				rax, rbx
	sub				rax, recurDeg_d
	add				rax, offSet_d

	; (j < 0) j += recurDeg_d
	; j = j + (mask & recurDeg_d)
	mov				r12, rax
	sar				r12, 63
	and				r12, recurDeg_d
	add				rax, r12

	; x = state_array[j] ^ xA
	shl				rax, 4
	vmovdqa				xmm0, [rdi+rax]
	shr				rax, 4
	vpxor				xmm0, xmm0, xmm1

	; state_array[k++] = x
	shl				rbx, 4
	vmovdqa				[rdi+rbx], xmm0
	shr				rbx, 4
	inc				rbx

	; (k >= recurDeg_d) k = 0
	; k = k - mask & k (mask = 0xf or 0x0)
	mov				r12, rbx
	sub				r12, recurDeg_d
	sar				r12, 63
	not				r12
	and				r12, rbx
	sub				rbx, r12

	; save state index
	mov				dword [rsi], ebx

	; tempering
	vpsrld				xmm1, xmm0, tempU_d
	vpxor				xmm0, xmm0, xmm1
	vpslld				xmm1, xmm0, tempS_d
	vpand				xmm1, xmm1, xmm11
	vpxor				xmm0, xmm0, xmm1
	vpslld				xmm1, xmm0, tempT_d
	vpand				xmm1, xmm1, xmm12
	vpxor				xmm0, xmm0, xmm1
	vpsrld				xmm1, xmm0, tempL_d
	vpxor				xmm0, xmm0, xmm1

	; returns result at res
	shl				r8, 4
	vmovdqa				[rdx+r8], xmm0
	shr				r8, 4

	inc				r8
	cmp				r8, rcx
	jb				_L0x02

_end_random_uint32__m128:
	pop				r12
	pop				rbx
	mov				rsp, rbp
	pop				rbp
	ret

_initialize_state__m512:
	push				rbp
	mov				rbp, rsp
	and				rsp, -16
	push				rbx

	; 16 bits of 1
	mov				ebx, 0xffff
	kmovw				k1, ebx

	; move 16 uint32 seeds into zmm0
	vmovdqa32			zmm0 {k1}{z}, [rdx]

	; state_array[511:0] = 16 initial seeds
	vmovdqa32			[rdi], zmm0
	
	; propagate initMul_d to zmm2
	mov				ecx, initMul_d
	vpbroadcastd			zmm2 {k1}{z}, ecx

	; 1s and state_indexes
	mov				ecx, 1
	vpbroadcastd			zmm3 {k1}{z}, ecx
	vmovdqa32			zmm4 {k1}{z}, zmm3

	xor				rbx, rbx
	xor				rcx, rcx
	add				rbx, 64
	mov				rcx, recurDeg_d
	sal				rcx, 6

_L0x03:
	; seed >> (genRange_d - 2)
	vpsrld				zmm1 {k1}{z}, zmm0, genRange_d

	; seed ^ (seed >> (genRange_d - 2))
	vpxord				zmm0 {k1}{z}, zmm0, zmm1

	; initMul_d * (seed ^ (seed >> (genRange_d)))
	vpmulld				zmm0, zmm0, zmm2

	; plus i
	vpaddd				zmm0 {k1}{z}, zmm0, zmm4

	; save
	vmovdqa32			[rdi+rbx], zmm0

	; update
	vpaddd				zmm4 {k1}{z}, zmm4, zmm3
	add				rbx, 64
	cmp				rbx, rcx
	jbe				_L0x03

	mov				dword [rsi], 0

_end_initialize_state__m512:
	pop				rbx
	mov				rsp, rbp
	pop				rbp
	ret

_random_uint32__m512:
	push				rbp
	mov				rbp, rsp
	and				rsp, -16
	push				rbx
	push				r12

	; init bitmasks
	mov				ebx, 0xffff
	kmovw				k1, ebx

	; init other const
	mov				ebx, UMASK_d
	vpbroadcastd			zmm2 {k1}{z}, ebx
	mov				ebx, LMASK_d
	vpbroadcastd			zmm3 {k1}{z}, ebx
	mov				ebx, twistCoef_d
	vpbroadcastd			zmm4 {k1}{z}, ebx
	mov				ebx, tempB_d
	vpbroadcastd			zmm11 {k1}{z}, ebx
	mov				ebx, tempC_d
	vpbroadcastd			zmm12 {k1}{z}, ebx

	xor				rax, rax
	xor				rbx, rbx
	xor				r8, r8
	xor				r12, r12

_L0x04:
	; get the state index, k
	mov				ebx, dword [rsi]

	; j
	mov				rax, rbx
	sub				rax, recurDeg_d
	inc				rax

	; (j < 0) j += recurDeg_d
	mov				r12, rax
	sar				r12, 63
	and				r12, recurDeg_d
	add				rax, r12

	; state_array[k]
	shl				rbx, 6
	vmovdqa32			zmm0 {k1}{z}, [rdi+rbx]
	shr				rbx, 6
	; state_array[j]
	shl				rax, 6
	vmovdqa32			zmm1 {k1}{z}, [rdi+rax]
	shr				rax, 6

	; state_arr[k] & UMASK_d
	vpandd				zmm0 {k1}{z}, zmm0, zmm2
	; state_arr[j] & LMASK_d
	vpandd				zmm1 {k1}{z}, zmm1, zmm3
	; x = (state_arr[k] & UMASK_d | state_arr[j] & LMASK_d)
	vpord				zmm0 {k1}{z}, zmm0, zmm1

	; xA = x >> 1
	vpsrld				zmm1 {k1}{z}, zmm0, 1
	; _mask = x & 0x00000001 -> only the LSB of x matters
	vpslld				zmm5 {k1}{z}, zmm0, 31
	vpsrad				zmm5 {k1}{z}, zmm5, 31
	vpmovd2m			k2, zmm5

	; xA filter non-zero using k2 mask
	; xA ^= twistCoef_d
	vpxord				zmm1 {k2}, zmm1, zmm4

	; j = k - recurDeg_d + offSet_d
	mov				rax, rbx
	sub				rax, recurDeg_d
	add				rax, offSet_d

	; (j < 0) j += recurDeg_d
	; j = j + (mask & recurDeg_d)
	mov				r12, rax
	sar				r12, 63
	and				r12, recurDeg_d
	add				rax, r12

	; x = state_array[j] ^ xA
	shl				rax, 6
	vmovdqa32			zmm0 {k1}{z}, [rdi+rax]
	shr				rax, 6
	vpxord				zmm0 {k1}{z}, zmm0, zmm1

	; state_array[k++] = x
	shl				rbx, 6
	vmovdqa32			[rdi+rbx], zmm0
	shr				rbx, 6
	inc				rbx

	; (k >= recurDeg_d) k = 0
	; k = k - mask & k (mask = 0xf or 0x0)
	mov				r12, rbx
	sub				r12, recurDeg_d
	sar				r12, 63
	not				r12
	and				r12, rbx
	sub				rbx, r12

	; save state index
	mov				dword [rsi], ebx

	; tempering
	vpsrld				zmm1 {k1}{z}, zmm0, tempU_d
	vpxord				zmm0 {k1}{z}, zmm0, zmm1
	vpslld				zmm1 {k1}{z}, zmm0, tempS_d
	vpandd				zmm1 {k1}{z}, zmm1, zmm11
	vpxord				zmm0 {k1}{z}, zmm0, zmm1
	vpslld				zmm1 {k1}{z}, zmm0, tempT_d
	vpandd				zmm1 {k1}{z}, zmm1, zmm12
	vpxord				zmm0 {k1}{z}, zmm0, zmm1
	vpsrld				zmm1 {k1}{z}, zmm0, tempL_d
	vpxord				zmm0 {k1}{z}, zmm0, zmm1

	; returns result at res
	shl				r8, 6
	vmovdqa32			[rdx+r8], zmm0
	shr				r8, 6

	inc				r8
	cmp				r8, rcx
	jb				_L0x04

_end_random_uint32__m512:
	pop				r12
	pop				rbx
	mov				rsp, rbp
	pop				rbp
	ret


