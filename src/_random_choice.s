section .note.GNU-stack

section .rodata
	align 16

section .text
	
	global _random_choice

_random_choice_noshuffle__512:
	push			rbp
	mov			rbp, rsp
	and			rsp, -16
	
	; allocate stack space for cdf
	; same size as pdf
	xor			rax, rax
	mov			eax, edx
	sal			eax, 2
	sub			rsp, rax

	push			rbx
	push			r12

	mov			r10, rbp
	sub			r10, rax
	lea			rbx, qword [r10]

	xor			rax, rax
	xor			r10, r10

_end_random_choice_noshuffle__512:
	pop			r12
	pop			rbx
	mov			rsp, rbp
	pop			rbp

