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


;  Expected latency: (n//16) * 94
;  1
;  3.5 + 4 + 1 + 4 + 14*(1+4) + 3 + 1 + 3.5 + 1 = 90
;  1 + 1 + 1 = 3
;  (n:latency) -> (<=16:94) (17-32:188) (33-48:282)
;  e.g.
;  	vxorps			zmm1 {k1}{z}, zmm1, zmm1
; 
;._dummyLabel1:
;  	_cumsumd__512		zmm0, zmm1, xmm0, rsi, r10, rbx
;       inc			r10
;       cmp			r10, 
;       jb			_dummyLabel1

%macro _cumsumd__512 6
        vmovaps                 %1 {k1}{z}, [%4+%5]
	vaddps			%1 {k1}{z}, %1, %2
        vpsrldq                 %2, %1, dSize_byte
        vaddps                  %1 {k1}{z}, %1, %2
%rep d__512-2
        vpsrldq                 %2, %2, dSize_byte
        vaddps                  %1 {k1}{z}, %1, %2
%endrep
	vbroadcastss		%2 {k1}{z}, %3
	sal			%5, 6
	vmovaps			[%6+%5], %1
	sar			%5, 6
%endmacro


;  Expected latency: (n//8) * 55
;  1
;  3.5 + 4 + 1 + 4 + 6*(1+4) + 3 + 1 + 3.5 + 1 = 51
;  1 + 1 + 1 = 3
;  (n:latency) -> (<=8:55) (9-16:110) (17-24:165) (25-32:220)
;  e.g.
;  	vxorps			ymm1 {k1}{z}, ymm1, ymm1
; 
;._dummyLabel1:
;  	_cumsumd__256		ymm0, ymm1, xmm0, rsi, r10, rbx
;       inc			r10
;       cmp			r10, 
;       jb			_dummyLabel1

%macro _cumsumd__256 6
        vmovaps                 %1 {k1}{z}, [%4+%5]
	vaddps			%1 {k1}{z}, %1, %2
        vpsrldq                 %2, %1, dSize_byte
        vaddps                  %1 {k1}{z}, %1, %2
%rep d__256-2
        vpsrldq                 %2, %2, dSize_byte
        vaddps                  %1 {k1}{z}, %1, %2
%endrep
	vbroadcastss		%2 {k1}{z}, %3
	sal			%5, 5
	vmovaps			[%6+%5], %1
	sar			%5, 5
%endmacro


;  Expected latency: (n//4) * 35
;  1
;  3.5 + 4 + 1 + 4 + 2*(1+4) + 3 + 1 + 3.5 + 1 = 31
;  1 + 1 + 1 = 3
;  (n:latency) -> (<=4:35) (5-8:70) (9-12:105) (13-16:140)
;  e.g.
;  	vxorps			xmm1 {k1}{z}, xmm1, xmm1
; 
;._dummyLabel1:
;  	_cumsumd__128		xmm0, xmm1, xmm0, rsi, r10, rbx
;       inc			r10
;       cmp			r10, 
;       jb			_dummyLabel1

%macro _cumsumd__128 6
        vmovaps                 %1 {k1}{z}, [%4+%5]
	vaddps			%1 {k1}{z}, %1, %2
        vpsrldq                 %2, %1, dSize_byte
        vaddps                  %1 {k1}{z}, %1, %2
%rep d__128-2
        vpsrldq                 %2, %2, dSize_byte
        vaddps                  %1 {k1}{z}, %1, %2
%endrep
	vbroadcastss		%2 {k1}{z}, %3
	sal			%5, 4
	vmovaps			[%6+%5], %1
	sar			%5, 4
%endmacro


%endif
