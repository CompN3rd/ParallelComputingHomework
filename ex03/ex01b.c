#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h> 
#endif

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>


#ifdef WIN32
double LIToSecs(LARGE_INTEGER * L)
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return ((double)L->QuadPart / (double)frequency.QuadPart);
}
double get_time()
{
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return LIToSecs(&time);
}
#else
double get_time()
{
	struct timeval time;
	gettimeofday(&time, NULL);
	return (double)time.tv_sec + (time.tv_usec / 1000000.0);
}
#endif


int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);

	int whoAmI;
	int worldSize;
	int counter = 1;

	MPI_Status stat;

	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	double start_time;
	double end_time;
	float runtime;
	int size;

	for (size = 1000; size <= 10000; size += 1000)
	{
		if (whoAmI == 0)
		{
			char* x = (char*)malloc(size * 1024 * sizeof(char));
			start_time = get_time();
			for (counter = 1; counter < worldSize; counter++)
			{
				MPI_Send(x, size * 1024, MPI_CHAR, counter, 0, MPI_COMM_WORLD);
			}
			end_time = get_time();
			runtime = end_time - start_time;
			printf("%i kb, %f s, %f kbs\n", size, runtime, (size / runtime));
			fflush(stdout);
			free(x);
		}
		else
		{
			char* x = (char*)malloc(size * 1024 * sizeof(char));
			MPI_Recv(x, size * 1024, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &stat);
			free(x);
		}
	}

	MPI_Finalize();
	return 0;
}
