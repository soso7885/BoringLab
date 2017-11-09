/* EE277A SIMD Lab Assignment
 *
 *
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

/*
	Original Version
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
	
	/* compare one pair of 16x16 blocks */
	unsigned int total = 0;
	for (int row = 0; row < 16; row++) {
		uint8x16_t vt_img1, vt_img2, vt_sum;
		uint8_t sum[16];
		
		vt_img1 = vld1q_u8(im1_p);	// load img1
		vt_img2 = vld1q_u8(im2_p);	// load img2
		vt_sum = vabdq_u8(vt_img1, vt_img2);	// abs
		
		vst1q_u8(sum, vt_sum);	// store back

		// loop unrolling
		total += sum[0];
		total += sum[1];		
		total += sum[2];		
		total += sum[3];		
		total += sum[4];		
		total += sum[5];		
		total += sum[6];		
		total += sum[7];		
		total += sum[8];		
		total += sum[9];		
		total += sum[10];		
		total += sum[11];		
		total += sum[12];		
		total += sum[13];		
		total += sum[14];		
		total += sum[15];		
/*
		total += vgetq_lane_u8(ay_sum, 0);
		total += vgetq_lane_u8(ay_sum, 1);
		total += vgetq_lane_u8(ay_sum, 2);
		total += vgetq_lane_u8(ay_sum, 3);
		total += vgetq_lane_u8(ay_sum, 4);
		total += vgetq_lane_u8(ay_sum, 5);
		total += vgetq_lane_u8(ay_sum, 6);
		total += vgetq_lane_u8(ay_sum, 7);
		total += vgetq_lane_u8(ay_sum, 8);
		total += vgetq_lane_u8(ay_sum, 9);
		total += vgetq_lane_u8(ay_sum, 10);
		total += vgetq_lane_u8(ay_sum, 11);
		total += vgetq_lane_u8(ay_sum, 12);
		total += vgetq_lane_u8(ay_sum, 13);
		total += vgetq_lane_u8(ay_sum, 14);
		total += vgetq_lane_u8(ay_sum, 15);
*/		
		im1_p += COLS;
		im2_p += COLS;
	}

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


