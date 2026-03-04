#ifndef __MT19337_SIMD__
#define __MT19337_SIMD__

//#include <stdint.h>

#define recurDeg_d 624
#define offSet_d 397
#define genRange_d 32
#define twistVal_d 31
#define UMASK_d (0xffffffffUL << twistVal_d)
#define LMASK_d (0xffffffffUL >> (genRange_d - twistVal_d))
#define twistCoef_d 0x9908b0dfUL
#define tempU_d 11
#define tempS_d 7
#define tempT_d 15
#define tempL_d 18
#define tempB_d 0x9d2c5680UL
#define tempC_d 0xefc60000UL
#define initMul_d 1812433253UL

typedef struct
{
	unsigned int state_array[recurDeg_d<<2] __attribute__((aligned(16)));
	int state_index;
} state__m128;

typedef struct
{
	unsigned int state_array[recurDeg_d<<4] __attribute__((aligned(64)));
	int state_index;
} state__m512;

extern void _initialize_state__m128
(
	unsigned int* starting_ptr
	, int* idx
	, unsigned int* seed
);

extern void _random_uint32__m128
(
	unsigned int* starting_ptr
	, int* idx
	, unsigned int* res
	, int num_blocks
);

extern void _initialize_state__m512
(
	unsigned int* starting_ptr
	, int* idx
	, unsigned int* seed
);

extern void _random_uint32__m512
(
	unsigned int* starting_ptr
	, int* idx
	, unsigned int* res
	, int num_blocks
);

#endif
