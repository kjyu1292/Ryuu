%include "BLOCKS.inc"
%include "CUMSUM_FUNCTION.inc"

section .note.GNU-stack

section .rodata
	align 16

section .text
	
	global _random_choice_noshuffle

_random_choice_noshuffle:
	push			rbp
	mov			rbp, rsp
	and			rsp, -16

	; resolve info, padded:actual
	xor			rax, rax
	;			actual, padded
	mov			eax, edx
	shr			rdx, 32
	xor			r11, r11
	mov			r11d, r8d
	shr			r8, 32
	
	; allocate stack space for cdf
	; same size as padded pdf
	sal			rdx, 2
	sub			rsp, rdx
	push			rbx
	push			r12

	; load stack address
	mov			r10, rbp
	sub			r10, rdx
	lea			rbx, [r10]
	sar			rdx, 2

	; set mask
	mov			r10d, 0xffff
	kmovw			k1, r10d
	xor			r10, r10

	_mask_cumsumd__512      ebx, k2, k3, k4, k5, zmm3, zmm4, zmm5, zmm6
	vxorps                  zmm7 {k1}{z}, zmm7, zmm7
	mov                     r11, rdx 
	sub                     r11, d__512

._L0x01:
	_cumsumd__512           zmm0, zmm1, zmm2, k2, k3, k4, k5, zmm3, zmm4, zmm5, zmm6, rsi, r10, rcx, zmm7
	add                     r10, d__512
	cmp                     r10, r11 
	jb                      ._L0x01
	
	sal			r10, d_shift
	_cumsumd_1__512         zmm0, zmm1, zmm2, k2, k3, k4, zmm3, zmm4, zmm5, rsi, r10
	vaddps                  zmm0 {k1}{z}, zmm0, zmm7
	vmovaps                 [rcx+r10], zmm0
	sar                     r10, d_shift

_end_random_choice_noshuffle:
	pop			r12
	pop			rbx
	mov			rsp, rbp
	pop			rbp
	ret







