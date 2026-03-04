#ifndef __RANDOM__
#define __RANDOM__

/*
   states: normalized bits from the prng
   pdf: probability density function
   pdf_info: [64:33,32:0] - [padded_size,actual_size]
   res: result array, can be any sizes
   res_info: same as pdf_info
*/
extern void _random_choice_noshuffle
(
 	unsigned int* states
	, float* pdf
	, long pdf_info
	, float* res		// og is uint, switch to float to test the midpoint cdf
	, long res_info
);

#endif
