#include <stdio.h>
#include <arm_neon.h>

void add3(uint8x16_t *data) 
{
	// uint8_t three[16] = {3, 3, ....}
	uint8x16_t three = vmovq_n_u8(3);

	/* uint8_t data[16] = {x+=3, x+=3, x+=3...}*/
	*data = vaddq_u8(*data, three);
}

void print_uint8(uint8x16_t data, const char *name)
{
	static uint8_t p[16];

	// store data to p	
	vst1q_u8(p, data);
	
	printf("%s = ", name);
	for(int i = 0; i < 16; ++i)
		printf("%d ", p[i]);
	printf("\n");
}

int main(void)
{
	const uint8_t org_data[] = {
				1, 2, 3, 4, 
				5, 6, 7, 8,
				9, 10, 11, 12, 
				13, 14, 15, 16
			};

	// uint8_t data[16]
	uint8x16_t data;
	
	// load org_data to data
	data = vld1q_u8(org_data);
	
	print_uint8(data, "orginal data");
	
	add3(&data);
	
	print_uint8(data, "new data");
	
	return 0;
}
