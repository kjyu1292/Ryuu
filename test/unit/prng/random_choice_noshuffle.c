#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "MT19937.h"
#include "RANDOM.h"

int main(int argc, char* argv[]) {

	assert(argc >= 3);

        int num_generations = atoi(argv[1]);
	assert(num_generations % 16 == 0);
	
	long pdf_actual = atoi(argv[2]);
	assert(pdf_actual < 0xFFFFF00);

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
    
        int num_blocks_m512 = num_generations / 16;
        unsigned int* pre_computed_generations = (unsigned int*) aligned_alloc(
                64, (num_generations * sizeof(int) + 63) & ~63 
        );
	_random_uint32__m512(st_ptr, idx_ptr, pre_computed_generations, num_blocks_m512);

	long pdf_padded = (pdf_actual + 15) & ~15;
	float pdf[(int) pdf_padded] __attribute__((aligned(64)));
	register int _dummy = (int) pdf_actual;
	for (register int i = 0; i < (int) pdf_actual; i++) 
	{
		pdf[i] = (float) 1/_dummy; 
	}
	long pdf_info = pdf_actual + (pdf_padded << 32);

	long res_actual = pdf_actual;
	long res_padded = (res_actual + 15) & ~15;
	float res[(int) res_padded] __attribute__((aligned(64)));
	for (register int i = 0; i < (int) res_padded; i++) 
	{
		res[i] = 0.0f; 
	}
	long res_info = res_actual + (res_padded << 32);
	assert(pdf_info == res_info);
	_random_choice_noshuffle(
		pre_computed_generations
		, &(pdf[0]), pdf_info
		, &(res[0]), res_info
	);

	for (register int i = 0; i < (int) res_actual; i++)
	{
		printf(
			"pdf[%2d] = %.7f | res[%2d] = %.7f\n"
			, i, pdf[i]
			, i, res[i]
		);
	}

	free(pre_computed_generations);

	return 0;
}
