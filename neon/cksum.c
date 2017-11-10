#include <stdio.h>
#include <assert.h>
#include <stdint.h>	
#include <sys/time.h>

#define SIZE	16384

int32_t cksum(uint8_t *src, int size)
{
	int32_t sum = 0;
	struct timeval start, end, diff;
	
	gettimeofday(&start, NULL);

	for(int i = 0; i < size; ++i)
		sum += src[i];
	
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
