#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <mpi.h>
#include <assert.h>
#include <math.h>

#define COLS	1000
#define ROWS	1000
// largest permitted change in temp 
#define MAX_TEMP_ERROR 0.01

double Temp[ROWS+2][COLS+2];      // Temp grid
double Temp_last[ROWS+2][COLS+2]; // Temp grid from last iteration

/*
 * initialize plate and boundary conditions
 * Temp_last is used to to start first iteration
 */
void initialize(void)
{
	for(int i = 0; i <= ROWS+1; i++){
		for (int j = 0; j <= COLS+1; j++){
			Temp_last[i][j] = 0.0;
		}
	}

	// these boundary conditions never change throughout run

	// set left side to 0 and right to a linear increase
	for(int i = 0; i <= ROWS+1; i++) {
		Temp_last[i][0] = 0.0;
		Temp_last[i][COLS+1] = (100.0/ROWS)*i;
	}

	// set top to 0 and bottom to linear increase
	for(int j = 0; j <= COLS+1; j++) {
		Temp_last[0][j] = 0.0;
		Temp_last[ROWS+1][j] = (100.0/COLS)*j;
	}
}

// print diagonal in bottom right corner where most action is
void track_progress(int iteration)
{
	printf("---------- Iteration number: %d ------------\n", iteration);
	for(int i = ROWS-5; i <= ROWS; i++) {
		printf("[%d,%d]: %5.2f  ", i, i, Temp[i][i]);
	}
	printf("\n");
}

// the main algorithm
double _calc_avg(int row_start, int row_end,
					int col_start, int col_end)
{
	double dt = 0.0;

	for(int i = row_start; i <= row_end; i++) {
		for(int j = col_start; j <= col_end; j++) {
			Temp[i][j] = 0.25 * (Temp_last[i+1][j] + Temp_last[i-1][j] +
								Temp_last[i][j+1] + Temp_last[i][j-1]);
		}
	}
	
	for(int i = row_start; i <= row_end; i++) {
		for(int j = col_start; j <= col_end; j++) {
			dt = fmax( fabs(Temp[i][j]-Temp_last[i][j]), dt );
			Temp_last[i][j] = Temp[i][j];
		}
	}
	return dt;
}

enum processor {
	MASTER = 0,
	NODE_1,
	NODE_2,
	NODE_3
};

int main(int argc, char **argv)
{
	int max_iterations; // number of iterations
	int iteration = 0; // current iteration
	double dt = 100.0; // largest change in t
	double max_dt;
	struct timeval start_time, stop_time, elapsed_time;	
	/* mpi stuff */
	char hostname[MPI_MAX_PROCESSOR_NAME];
	int nr_task, taskid, name_len;

	MPI_Init(NULL, NULL);

	MPI_Comm_size(MPI_COMM_WORLD, &nr_task);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Get_processor_name(hostname, &name_len);

	if(taskid == MASTER){
		printf("%s: %d process is running!\n", hostname, nr_task);
		printf("Maximum iterations [100-4000]?\n");
		scanf("%d", &max_iterations);
	}

	// everybody got the max_iteration	
	MPI_Bcast(&max_iterations, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	if(max_iterations > 4000 || max_iterations < 100){
		if(taskid == MASTER)
			printf("The range of Maximum iterations is 100~4000\n");
		goto exit;
	}

	// everybody do the initail	
	initialize();

	if(taskid == MASTER)
		gettimeofday(&start_time, NULL); // Unix timer

	/*------------------------ MPI work -------------------------*/
	while(dt > MAX_TEMP_ERROR && iteration <= max_iterations){
		switch(taskid){
			case MASTER:
				dt = _calc_avg(ROWS/2, ROWS, COLS/2, COLS);
				break;
			case NODE_1:
				dt = _calc_avg(ROWS/2, ROWS, 1, COLS/2-1);
				break;
			case NODE_2:
				dt = _calc_avg(1, ROWS/2-1, 1, COLS/2-1);
				break;
			case NODE_3:
				dt = _calc_avg(1, ROWS/2-1, COLS/2, COLS);
				break;
			default:
				break;
		}

		if(taskid == MASTER){
			//periodically print test values
			if((iteration % 100) == 0)
				track_progress(iteration);
		}
	
		MPI_Barrier(MPI_COMM_WORLD);
		// find the maxium_dt from each node and stored to dt
		MPI_Allreduce(&dt, &max_dt, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
		dt = max_dt;
		MPI_Barrier(MPI_COMM_WORLD);

		iteration++;
	}
	/*-----------------------------------------------------------------*/

	if(taskid == MASTER){
		gettimeofday(&stop_time, NULL);
		timersub(&stop_time, &start_time, &elapsed_time);
		printf("\nMax error at iteration %d was %f\n", iteration-1, dt);
		printf("Total time was %f seconds.\n", 
				elapsed_time.tv_sec+elapsed_time.tv_usec/1000000.0);
	}

exit:
	MPI_Finalize();

	return 0;
}
