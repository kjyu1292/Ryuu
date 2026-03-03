#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "MT19937_SIMD.h"

int main() {

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	unsigned int seeds[4] __attribute__((aligned(16)));
	unsigned int seed = (unsigned int) \
		(ts.tv_sec*1000000000+ts.tv_nsec);
	seed = seed;
	
	for (register int i = 0; i < 4; i++)
	{
		seeds[i] = 19650218; // + (seed & i) + (seed >> i);
	}
 
	state__m128 stack_state;
	state__m128* state = &stack_state;
	unsigned int* st_ptr = &(state->state_array[0]);
	int* idx_ptr = &(state->state_index);
	_initialize_state__m128(st_ptr, idx_ptr, &(seeds[0]));

	return 0;

}


















