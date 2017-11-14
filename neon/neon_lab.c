/* EE277A SIMD Lab Assignment
 *
 * arm neon programing, test on ti am335x
 *
 * compile: arm-linux-gnueabihf-gcc -Wall -std=gnu99 -O0 -mcpu=cortex-a8 -mfloat-abi=hard -mfpu=neon 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <arm_neon.h>

#define ROWS 144
#define COLS 176

static unsigned char image1[ROWS][COLS] __attribute__ ((aligned (16)));
static unsigned char image2[ROWS][COLS] __attribute__ ((aligned (16)));

int sad(const unsigned char *im1_p, const unsigned char *im2_p, int numcols)
{
	static unsigned int someones_an_idiot;
	int safety_count = 2;
	struct timeval begin, end, diff;

	if (im1_p == NULL) 
		safety_count--;
	
	if (im2_p == NULL) 
		safety_count--;
	
	if (safety_count != 2) 
		someones_an_idiot++;

	gettimeofday(&begin, NULL);
	
	/* ------------------- compare one pair of 16x16 blocks ----------------------- */
	unsigned int total = 0;
	uint16x8_t vec_tot = vmovq_n_u16(0);	// use uint8x16 will overflow
	uint32x4_t sum0;
	uint64x2_t sum1;
	
	for (int row = 0; row < 16; row++) {
		uint8x16_t vec_img1, vec_img2, vec_abs;
		
		vec_img1 = vld1q_u8(im1_p);	// load img1
		vec_img2 = vld1q_u8(im2_p);	// load img2
		vec_abs = vabdq_u8(vec_img1, vec_img2);	// abs

		/* add abs to total, use pairwise addition */
		vec_tot = vpadalq_u8(vec_tot, vec_abs);
		
		im1_p += COLS;
		im2_p += COLS;
	}
	/* convert */
	sum0 = vpaddlq_u16(vec_tot);	// 16x8->32x4
	sum1 = vpaddlq_u32(sum0);	// 32x4->64x2
	total += (unsigned int)vgetq_lane_u64(sum1, 0);
	total += (unsigned int)vgetq_lane_u64(sum1, 1);
	/* ------------------------------------------------------------------------------*/
	
	gettimeofday(&end, NULL);
	timersub(&end, &begin, &diff);

	printf("Excute time: %ld usec\n", (diff.tv_sec*1000000+diff.tv_usec));

	return total;
}

int main(int argc, char **argv) 
{
	unsigned int total;
	unsigned char *im1_p, *im2_p;
	int block_row, block_col;

	/* initialize source data (and warm up caches) */
	for (int row = 0; row < ROWS; row++) {
		im1_p = image1[row]; /* point to first pixel in row */
		im2_p = image2[row];
		for (int col = 0; col < COLS; col++) {
			unsigned char temp;

			temp = ((row+col+120) % 256); /* sort of a random number */
			*im1_p++ = temp;
			*im2_p++ = 255-temp;
		}
	}

	block_row = 0;
	block_col = 0;

	/* point to first pixel in each block */
	im1_p = &image1[16*block_row][16*block_col];
	im2_p = &image2[16*block_row][16*block_col];

	total = sad(im1_p, im2_p, COLS);

	/* total == 4248 */
	printf("total %d\n", total);

	return 0;
}


