#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "MT19937_SIMD.h"

int main() {

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	unsigned int seeds[16] __attribute__((aligned(64)));
	unsigned int seed = (unsigned int) \
		(ts.tv_sec*1000000000+ts.tv_nsec);
	seed = seed;
	
	for (register unsigned int i = 0; i < 16; i++)
	{
		seeds[i] = 19650218; // + (seed & i) + (seed >> i);
	}
 
	state__m512 stack_state;
	state__m512* state = &stack_state;
	unsigned int* st_ptr = &(state->state_array[0]);
	int* idx_ptr = &(state->state_index);
	_initialize_state__m512(st_ptr, idx_ptr, &(seeds[0]));

	int NUM = 626;
	unsigned int result[16*NUM] __attribute__((aligned(64)));
	for (register int i = 0; i < 16*NUM; i++) { result[i] = 0; }
	_random_uint32__m512(st_ptr, idx_ptr, result, NUM);

	return 0;

}


