%include "BLOCKS.inc"
%include "CUMSUM_FUNCTION.inc"

section .note.GNU-stack

section .rodata
	align 16

section .text
	
	global __random_choice_noshuffle

_random_choice_noshuffle:
	push			rbp
	mov			rbp, rsp
	and			rsp, -16
	push			rbx
	push			r12

	; resolve info, padded:actual
	xor			rax, rax
	;			actual, padded
	mov			eax, edx
	shr			rdx, 32
	xor			rbx, rbx
	mov			ebx, r8d
	shr			r8, 32
	
	; allocate stack space for cdf
	; same size as padded pdf
	sal			rdx, 2
	sub			rsp, rdx
	sar			rdx, 2

	; load stack address
	mov			r10, rbp
	sub			r10, rax
	lea			rbx, [r10]

	; set mask
	mov			r10d, 0xffff
	kmovw			k1, r10d
	xor			r10, r10

	vxorps			zmm1 {k1}{z}, zmm1, zmm1

._L0x01:
	_cumsumd__512		zmm0, zmm1, xmm0, rsi, r10, rcx
	inc			r10
	cmp			r10, rdx
	jb			._L0x01

_end_random_choice_noshuffle:
	pop			r12
	pop			rbx
	mov			rsp, rbp
	pop			rbp











