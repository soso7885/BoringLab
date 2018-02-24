#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define INPUT1 98.0
#define INPUT2 -89.0

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

static inline int _find_last_set_bit(uint32_t mts)
{
	return (31 - __builtin_clz(mts));
}

static inline void normalized(uint32_t *mts)
{
	*mts &= 0x007fffff;
}

static inline int underflow_fixed(uint32_t *mts)
{
	int fixed = 23 - _find_last_set_bit(*mts);
	
	*mts = *mts << fixed;
	
	return fixed;
}

static inline int overflow_fixed(uint32_t *mts)
{
	int fixed = _find_last_set_bit(*mts) - 23;
	
	*mts = *mts >> fixed;

	return fixed;
}

static inline int check_overflow(uint32_t mts)
{
	return ((mts & (0x01ffffff)) >> 23);
}

int main(void)
{
	my_float d1 = {0};
	my_float d2 = {0};
	my_float sum = {0};
	my_float check = {0};

	d1.f = INPUT1;
	d2.f = INPUT2;
	check.f = d1.f + d2.f;
	
	PRINTALL(d1);
	PRINTALL(d2);

	do{
		/* Check input is 0 or not */
		if(d1.parts.exp == 0){
			memcpy(&sum, &d2, sizeof(sum));
			break;
		}
		if(d2.parts.exp == 0){
			memcpy(&sum, &d1, sizeof(sum));
			break;
		}
		
		/*
		 * Step 1: exponent comparator 
		*/
		int diff = abs(take_exp(d1.parts.exp) - take_exp(d2.parts.exp));
		/*
		 * Step 2: mantissa alignment
		 * 		we take out the mts and unnormailzed it,
		 * 		its a good way to alignment and caculate
		*/
		uint32_t mts1 = d1.parts.mts | (1 << 23);
		uint32_t mts2 = d2.parts.mts | (1 << 23); 
		uint8_t sum_exp;

		if(d1.parts.exp < d2.parts.exp){
			mts1 = mts1 >> diff;
			sum_exp = d2.parts.exp;
		}else if (d2.parts.exp < d1.parts.exp){
			mts2 = mts2 >> diff;
			sum_exp = d1.parts.exp;
		}else{
			sum_exp = d1.parts.exp;
		}


		uint8_t sum_sig;
		/*
		 * Step 3: Compare the two aligned mantissas
		 * 			determine which is the smaller of the two
		 *	< no need while same sign >
		*/
		if(d1.parts.sig != d2.parts.sig){
			if(mts1 < mts2){
				mts1 = ~mts1 + 1;
				sum_sig = d2.parts.sig;
			}else{
				mts2 = ~mts2 + 1;
				sum_sig = d1.parts.sig;
			}
		}else{
			sum_sig = d1.parts.sig;
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
				printf("Underflow!\n");
				fixed = underflow_fixed(&sum_mts);
				sum_exp -= fixed;
				break;
			case 1:
				break;
			case 2:
			case 3:
				printf("Overflow!\n");
				fixed = overflow_fixed(&sum_mts);
				sum_exp += fixed;
				break;
		}
		normalized(&sum_mts);
	
		/* Write back */
		sum.parts.sig = sum_sig;
		sum.parts.exp = sum_exp;
		sum.parts.mts = sum_mts;
	}while(0);

	PRINTALL(sum);
	PRINTALL(check);
	
	return 0;
}
