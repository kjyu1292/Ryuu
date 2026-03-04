#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "MT19937.h"

int main(int argc, char* argv[])
{
	assert(argc >= 5);

	int NUM_BATCHES = 	atoi(argv[1]);
	int NUM_EPS 	= 	atoi(argv[2]);
        int d1          =       atoi(argv[3]);
        int d2          =       atoi(argv[4]);

	struct timespec ts; 
        clock_gettime(CLOCK_REALTIME, &ts);
	unsigned int seeds[4] __attribute__((aligned(16)));
        unsigned int seed = (unsigned int) \
		(ts.tv_sec*1000000000+ts.tv_nsec);
	seed = seed;
    
	for (register unsigned int i = 0; i < 4; i++)
	{
		seeds[i] = 19650218; // + (seed & i) + (seed >> i);
	}

        state__m128 stack_state;
        state__m128* state = &stack_state;
        unsigned int* st_ptr = &(state->state_array[0]);
        int* idx_ptr = &(state->state_index);
        _initialize_state__m128(st_ptr, idx_ptr, &(seeds[0]));
        
	int actual_n = d1 * d2;
	int padded_n = (actual_n + 3) + ~3;
	int num_blocks_m128 = padded_n / 4;
	int multiples = padded_n * 4;
	unsigned int* result = (unsigned int*) aligned_alloc(
		16, (multiples + 15) & ~15
	);
	
	struct timespec start, end;
	unsigned long long dur = 0;
	for (register int i = 0; i < NUM_BATCHES; i++)
	{
		clock_gettime(CLOCK_MONOTONIC, &start);
		for (register int j = 0; j < NUM_EPS; j++)
		{
			_random_uint32__m128(st_ptr, idx_ptr, result, num_blocks_m128);
		}
		clock_gettime(CLOCK_MONOTONIC, &end);
		dur += (unsigned long long) ( \
			(end.tv_sec - start.tv_sec) \
			* 1000000000 + (end.tv_nsec - start.tv_nsec)
		);
	}

	dur /= NUM_BATCHES;
	printf("Average per %d-run of size %dx%d: ", NUM_EPS, d1, d2);
	printf("%llu (nsec)\n", dur);
	printf("Average per op of size %dx%d: ", d1, d2);
	printf("%llu (nsec)\n", dur / NUM_EPS);

	free(result);

	return 0;
}



