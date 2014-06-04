#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <stdlib.h>   // do we need this?
#endif

#include <stdio.h>
#include <cstdlib>
#include <mpi.h>
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
	return (double) time.tv_sec + (time.tv_usec/1000000.0);
}
#endif



int main(int argc, char **argv) {

	MPI_Init(&argc, &argv);

	double	start_time,
			end_time,
			runtime,
			s,
			gflops;

	int		whoAmI,
			i,
			n;				// vector length

	double	*x,				
			*y;

	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);


	for( n = 1000 ; n <= 10000 ; n += 1000)
	{
		x = new double [n];
		y = new double [n];

		for(s=0.,i=0;i<n;i++)
		{
			x[i] = rand();
			y[i] = rand();
		}

		start_time = get_time();
		for(s=0.,i=0;i<n;i++) 
		{
			s += x[i] * y[i];
		}
		end_time = get_time();

		runtime = end_time - start_time;
		gflops = n*n / runtime / 1000000000;
		
		printf("GFLOPS: %f, Runtime: %f\n",gflops,runtime);
		//fflush(stdout);		
		
		delete[] x;
		delete[] y;
	}

	MPI_Finalize();
	return 0;
}
