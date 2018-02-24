#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bsd/stdlib.h>
#include <stdint.h>

typedef union {
	float f;
	struct {
		uint32_t mts : 23;
		uint32_t exp : 8;
		uint32_t sig : 1;
	} parts;
} my_float;

#define take_exp(exp)	(exp-127)

#define PRINTALL(var) \
	printf("=======================================\n\n");	\
	printf(#var" is %.4f\n", var.f);						\
	printf(#var"'s sign is %s\n", var.parts.sig? "1" : "0");\
	printf(#var"'s exponent is 0x%x(2^%d)\n", 				\
			var.parts.exp, take_exp(var.parts.exp));		\
	printf(#var"'s mantisa is 0x%x\n", var.parts.mts);		\
	printf("\n=======================================\n");

/*
 * Find the first set bit from left side
*/
static inline int find_last_set_bit(uint32_t mts)
{
	return (31 - __builtin_clz(mts));
}

static inline void normalized(uint32_t *mts)
{
	*mts &= 0x007fffff;
}

static inline int underflow_fixed(uint32_t *mts)
{
	int fixed = 23 - find_last_set_bit(*mts);
	
	*mts = *mts << fixed;
	
	return fixed;
}

static inline int overflow_fixed(uint32_t *mts)
{
	int fixed = find_last_set_bit(*mts) - 23;
	
	*mts = *mts >> fixed;

	return fixed;
}

static inline int check_overflow(uint32_t mts)
{
	return ((mts & (0x01ffffff)) >> 23);
}

#define is_zero(in) (in->parts.exp == 0)
static void ieee754_adder(my_float *in1, my_float *in2, my_float *out)
{
	/* Check input is 0 or not */
	if(is_zero(in1)){
		memcpy(out, in2, sizeof(my_float));
		return;
	}

	if(is_zero(in2)){
		memcpy(out, in1, sizeof(my_float));
		return;
	}
	
	/*
	 * Step 1: exponent comparator 
	*/
	int diff = abs(take_exp(in1->parts.exp) - take_exp(in2->parts.exp));
	/*
	 * Step 2: mantissa alignment
	 * 		we take out the mts and unnormailzed it,
	 * 		its a good way to alignment and caculate
	*/
	uint32_t mts1 = in1->parts.mts | (1 << 23);
	uint32_t mts2 = in2->parts.mts | (1 << 23); 
	uint8_t sum_exp;
	
	if(in1->parts.exp < in2->parts.exp){
		mts1 = mts1 >> diff;
		sum_exp = in2->parts.exp;
	}else if (in2->parts.exp < in1->parts.exp){
		mts2 = mts2 >> diff;
		sum_exp = in1->parts.exp;
	}else{
		sum_exp = in1->parts.exp;
	}

	uint8_t sum_sig;
	/*
	 * Step 3: Compare the two aligned mantissas 
	 * 			determine which is the smaller of the two
	 *	< no need while same sign >
	*/
	if(in1->parts.sig != in2->parts.sig){
		if(mts1 < mts2){
			mts1 = ~mts1 + 1;
			sum_sig = in2->parts.sig;
		}else{
			mts2 = ~mts2 + 1;
			sum_sig = in1->parts.sig;
		}
	}else{
		sum_sig = in1->parts.sig;
	}
		

	/*
	 * Step 4: mantissa adder
	*/
	uint32_t sum_mts = mts1 + mts2;
	/*
	 *	Step 5: Normalizer & Rounding
	 * 		We need to check the result is in overflow or not
	 *		(see bit 23 & 24)
	 *		four kind of situation:
	 *			0(00) -> means underflow, normalized and left shift!
	 *			1(01) -> no overflow, just normalized it
	 *			2(10) -> overflow, normalized and right shift it
	 *			3(11) -> overflow, same as situation 2
	 * 
	*/
	int fixed;
	switch(check_overflow(sum_mts)){
		case 0:
			fixed = underflow_fixed(&sum_mts);
			sum_exp -= fixed;
			break;
		case 1:
			break;
		case 2:
		case 3:
			fixed = overflow_fixed(&sum_mts);
			sum_exp += fixed;
			break;
	}
	normalized(&sum_mts);

	/* Write back */
	out->parts.sig = sum_sig;
	out->parts.exp = sum_exp;
	out->parts.mts = sum_mts;
}
#undef is_zero

int main(int argc, char **argv)
{
	my_float d1, d2, sum, check;

	if(argc < 2){
		printf("%s [test round]\n", argv[0]);
		return -1;
	}
	
	int round = atoi(argv[1]);
	int err = 0;

	for(int i = 1; i <= round; ++i){
		memset(&d1, 0, sizeof(my_float));
		memset(&d2, 0, sizeof(my_float));
		memset(&sum, 0, sizeof(my_float));
		memset(&check, 0, sizeof(my_float));

		d1.f = (float)(arc4random() % 12345);
		d2.f = (float)(arc4random() % 54321);

		check.f = d1.f + d2.f;

		printf("%d / %d round test ..................", i, round);

		ieee754_adder(&d1, &d2, &sum);
	
		if(memcmp(&sum, &check, sizeof(my_float))){
			printf(" Error !!   \n\n");
			PRINTALL(d1);
			PRINTALL(d2);
			PRINTALL(sum);
			PRINTALL(check);
			err = 1;
			break;
		}
			
		printf(" Pass !!        \r");
		fflush(stdout);
	}
	
	printf("\n");
	if(!err) printf("Passing %d test !!\n\n", round);
	
	return 0;
}
