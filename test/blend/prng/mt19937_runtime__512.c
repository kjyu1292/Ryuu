#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "MT19937_SIMD.h"

int main(int argc, char* argv[])
{
	assert(argc >= 5);

	int NUM_BATCHES = 	atoi(argv[1]);
	int NUM_EPS 	= 	atoi(argv[2]);
	int d1 		= 	atoi(argv[3]);
	int d2 		= 	atoi(argv[4]);
        
	struct timespec ts; 
        clock_gettime(CLOCK_REALTIME, &ts);
	unsigned int seeds[16] __attribute__((aligned(64)));
        unsigned int seed = (unsigned int) \
		(ts.tv_sec*1000000000+ts.tv_nsec);
        seed = seed;
    
        for (register unsigned int i = 0; i < 4; i++)
        {   
                seeds[i] = 19650218; // + (seed & i) + (seed >> i);
        }     
 
        state__m512 stack_state;
        state__m512* state = &stack_state;
        unsigned int* st_ptr = &(state->state_array[0]);
        int* idx_ptr = &(state->state_index);
        _initialize_state__m512(st_ptr, idx_ptr, &(seeds[0]));
        
	int actual_n = d1 * d2;
	int padded_n = (actual_n + 15) & ~15;
	int num_blocks_m512 = padded_n / 16;		// prolly redundant
	int multiples = padded_n * 4;
	unsigned int* result = (unsigned int*) aligned_alloc(
		64, (multiples + 63) & ~63
	);
	
	struct timespec start, end;
	unsigned long long dur = 0;
	for (register int i = 0; i < NUM_BATCHES; i++)
	{
		clock_gettime(CLOCK_MONOTONIC, &start);
		for (register int j = 0; j < NUM_EPS; j++)
		{
			_random_uint32__m512(st_ptr, idx_ptr, result, num_blocks_m512);
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



