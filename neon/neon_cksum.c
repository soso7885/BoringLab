#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <sys/time.h>
#include <arm_neon.h>

#define SIZE 16384

int32_t cksum(uint8_t *src, int size)
{
	int32_t sum = 0;
	struct timeval start, end, diff;
	uint16x8_t tmp0;
	uint32x4_t tot = vdupq_n_u32(0);
	uint64x2_t res;

	/* buffer size must be multiple of 8 */
	assert(size%8 == 0);

	gettimeofday(&start, NULL);

	for(int i = 0; i < size; i+=16, src += 16){
		uint8x16_t load;

		load = vld1q_u8(src);
		tmp0 = vpaddlq_u8(load);	//8x16->16x8
		tot = vpadalq_u16(tot, tmp0);	//16x8->32x4
	}
	
	res = vpaddlq_u32(tot);	//32x4->64x2

	sum += (int32_t)vgetq_lane_u64(res, 0);
	sum += (int32_t)vgetq_lane_u64(res, 1);

	gettimeofday(&end, NULL);
	timersub(&end, &start, &diff);
	
	printf("Excute time: %ld usec\n", (diff.tv_sec*1000000+diff.tv_usec));

	return sum;
}

int main(void)
{
	uint8_t buffer[SIZE];
	int32_t sum;

	buffer[0] = 1;	
	for(int i = 1; i < sizeof(buffer); ++i)
		buffer[i] = (buffer[i-1] << 1 | buffer[i-1]>>7);
	
	sum = cksum(buffer, sizeof(buffer));
	
	printf("Checksum = %d\n", sum);
	
	return 0;
}
