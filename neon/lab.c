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
//#include <arm_neon.h>

#define ROWS 144
#define COLS 176

#define TEST 16

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
	for (int row = 0; row < TEST; row++) {
		for (int col = 0; col < TEST; col++) {
			total += abs(*im1_p - *im2_p);
			im1_p = im1_p + 1 ; //= im1_p + 1; // increment image1 column ptr
			im2_p = im2_p + 1 ; //= im2_p + 1; // increment image2 column ptr
		}
		im1_p = im1_p +  (COLS - TEST);
		im2_p = im2_p +  (COLS - TEST);
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


