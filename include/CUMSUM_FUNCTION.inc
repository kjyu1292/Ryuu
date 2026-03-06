%ifndef __CUMSUM_FUNCTION_COST__
%define __CUMSUM_FUNCTION_COST__



%include "BLOCKS.inc"



;  If modded to the T, it would require some shenanigans
;  during compiling or assembling.
;  For general purposes, we can just compile everything
;  and apply some cmp/jmp for run-time sizes. This would, of course,
;  incur some overheads during run-time.
;  Below are the expected latencies for different compile-time
;  implementations to calculate the cumulative sum of an array
;  These are not functions, and the expected latencies were
;  calculated with actual code flow in mind.
;  *** There is domain crossing in these implementations.


      
;  Simple implementation
;  Expected latency: 8+14(n-1) -> n = 16 : 218
;  4 + 3 + 1 + (n-1) * (4 + 4 + 3 + 1 + 1 + 1) = 8+14(n-1)
;  Implementation:
;       vmovss                  xmm0 {k1}{z}, [rsi]
;       vmovss                  [rbx], xmm0
;       inc                     r10
;
;._dummyLabel1:
;       vmovss                  xmm1 {k1}{z}, [rsi+r10*4]
;       vaddss                  xmm0, xmm0, xmm1
;       vmovss                  [rbx+r10*4]. xmm0
;       inc                     r10
;       cmp                     r10,
;       jb                      _dummyLabel1



;  Description:
;    Setting up masks specifically for cumsum 16 doublewords ONCE
;  Expected latency:
;    6 mov | 3 kmovw k,r | 3 vpbroadcastd z,r
;  Implementation:
;  	_mask_cumsumd_1__512	ebx, k2, k3, k4, zmm3, zmm4, zmm5
;  	...
;	_cumsumd_1__512		...

%macro _mask_cumsumd_1__512 7
	mov			%1, 0b1111111111110000
	kmovw			%2, %1
	mov			%1, 0b1111111100000000
	kmovw			%3, %1
	mov			%1, 0b1111000000000000
	kmovw			%4, %1

	mov			%1, 0b10011
	vpbroadcastd		%5 {k1}{z}, %1
	mov			%1, 0b10111
	vpbroadcastd		%6 {k1}{z}, %1
	mov			%1, 0b11011
	vpbroadcastd		%7 {k1}{z}, %1
%endmacro



;  Description:
;    Perform cumulative sum on an array of <=16 doublewords
;    %1: base zmm to move array from memory, and do cumulative add
;    %2: placeholder zmm to perform shift and permutation on
;    %3: copy of base midway through the calculation
;    %4-9: fixed masks
;    %10: base address to load the array data
;    %11: offset/stepsize from the base address
;  Expected latency:
;    2 vmovaps z,m | 1 vmovaps z,z | 3 vpsrldq z,z,imm
;    7 vaddps z,z,z | 3 vpermt2ps z,z,z | 2 shift
;  Implementation:
;	_mask_cumsumd_1__512	ebx, k2, k3, k4, zmm3, zmm4, zmm5
;	sal			r10, d_shift
;	_cumsumd_1__512		zmm0, zmm1, zmm2, k2, k3, k4, zmm3, zmm4, zmm5, rsi, r10
;	vmovaps			[rcx+r10], zmm0
;	sar			r10, d_shift

%macro _cumsumd_1__512 11
	vmovaps			%1 {k1}{z}, [%10+%11]
	vpslldq			%2, %1, dSize_byte
	vaddps			%1 {k1}{z}, %1, %2
	vpslldq			%2, %2, dSize_byte
	vaddps			%1 {k1}{z}, %1, %2
	vpslldq			%2, %2, dSize_byte
	vaddps			%1 {k1}{z}, %1, %2

	vmovaps			%3 {k1}{z}, %1

	vpermt2ps		%2 {%4}{z}, %7, %3
	vaddps			%1 {k1}{z}, %1, %2
	vpermt2ps		%2 {%5}{z}, %8, %3
	vaddps			%1 {k1}{z}, %1, %2
	vpermt2ps		%2 {%6}{z}, %9, %3
	vaddps			%1 {k1}{z}, %1, %2
%endmacro



;  Description:
;    Setting up masks specifically for cumsum 16 doublewords with LOOP
;  Expected latency:
;    8 mov | 4 kmovw k,r | 4 vpbroadcastd z,r
;  e.g.
;  	_mask_cumsumd__512	ebx, k2, k3, k4, k5, zmm3, zmm4, zmm5, zmm6
;  	...
;	_cumsumd__512		...

%macro _mask_cumsumd__512 9
	mov			%1, 0b1111111111110000
	kmovw			%2, %1
	mov			%1, 0b1111111100000000
	kmovw			%3, %1
	mov			%1, 0b1111000000000000
	kmovw			%4, %1
	mov			%1, 0b1111111111111111
	kmovw			%5, %1

	mov			%1, 0b10011
	vpbroadcastd		%6 {k1}{z}, %1
	mov			%1, 0b10111
	vpbroadcastd		%7 {k1}{z}, %1
	mov			%1, 0b11011
	vpbroadcastd		%8 {k1}{z}, %1
	mov			%1, 0b11111
	vpbroadcastd		%9 {k1}{z}, %1
%endmacro



;  Description:
;    Perform cumulative sum on an array of >16 doublewords
;    %1: base zmm to move array from memory, and do cumulative add
;    %2: placeholder zmm to perform shift and permutation on
;    %3: copy of base midway through the calculation
;    %4-11: fixed masks
;    %12: base address to load the array data
;    %13: offset/stepsize from the base address
;    %14: base address to store the array data 
;  Expected latency:
;    2 vmovaps z,m | 1 vmovaps z,z | 3 vpsrldq z,z,imm
;    6 vaddps z,z,z | 3 vpermt2ps z,z,z | 2 shift
;  Implementation:
;	_mask_cumsumd__512	ebx, k2, k3, k4, k5, zmm3, zmm4, zmm5, zmm6
;	vxorps			zmm7 {k1}{z}, zmm7, zmm7
;	mov			r11, rdx
;	sub			r11, d__512
;
;  ._dummyLabel1:
;	_cumsumd__512		zmm0, zmm1, zmm2, k2, k3, k4, k5, zmm3, zmm4, zmm5, zmm6, rsi, r10, rcx, zmm7
;	add			r10, d__512
;	cmp			r10, r11
;	jb			._dummyLabel1
;
;	sal			r10, d_shift
;	_cumsumd_1__512		zmm0, zmm1, zmm2, k2, k3, k4, zmm3, zmm4, zmm5, rsi, r10
;	vaddps			zmm0 {k1}{z}, zmm0, zmm7
;	vmovaps			[rcx+r10], zmm0
;	sar			r10, d_shift

%macro _cumsumd__512 15
	sal			%13, d_shift
	vmovaps			%1 {k1}{z}, [%12+%13]
	vpslldq			%2, %1, dSize_byte
	vaddps			%1 {k1}{z}, %1, %2
	vpslldq			%2, %2, dSize_byte
	vaddps			%1 {k1}{z}, %1, %2
	vpslldq			%2, %2, dSize_byte
	vaddps			%1 {k1}{z}, %1, %2

	vmovaps			%3 {k1}{z}, %1

	vpermt2ps		%2 {%4}{z}, %8, %3
	vaddps			%1 {k1}{z}, %1, %2
	vpermt2ps		%2 {%5}{z}, %9, %3
	vaddps			%1 {k1}{z}, %1, %2
	vpermt2ps		%2 {%6}{z}, %10, %3
	vaddps			%1 {k1}{z}, %1, %2

	vaddps			%1 {k1}{z}, %1, %15
	vmovaps			[%14+%13], %1
	sar			%13, d_shift

	vpermt2ps		%15 {%7}{z}, %11, %1
%endmacro


%endif



















