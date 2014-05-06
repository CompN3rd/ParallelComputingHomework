#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>


double get_time()
{
	struct timeval time;
	gettimeofday(&time, NULL);
	return (double) time.tv_sec + (time.tv_usec/1000000.0);
}


int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);

	int whoAmI;
	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);
	printf("I am %d\n", whoAmI);

	MPI_Status stat;

	double start_time;
	double end_time;
	float runtime;
	int size = 1000;
	switch (whoAmI) {
	case 0: {
		char* x = malloc(size * 1024 * sizeof(char));
		start_time = get_time();
		MPI_Send(x, size * 1024, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
		MPI_Recv(x, 0, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &stat);
		end_time = get_time();
		runtime = end_time - start_time; 
		printf("%i kb,%f s,%f kbs\n", size, runtime, (size/runtime));
		free(x);
		break;
			}
	case 1: {
		char* x = malloc(size * 1024 * sizeof(char));
		MPI_Recv(x, size*1024, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &stat);
		MPI_Send(x, 0, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
		free(x);
		break;
			}
	default:
		printf("hello. i am your missing child! #%d\n", whoAmI);
	}

	MPI_Finalize();
	return 0;
}
