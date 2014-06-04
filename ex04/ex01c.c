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
#include <list>
#include <math.h> 


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

double inner_product(double *x, double *y, int begin, int end)
{
	double s=0;
	for(; begin < end ; begin++)
	{
		s += x[begin] * y[begin];
	}
	return s;
}

int main(int argc, char **argv) {

	MPI_Init(&argc, &argv);

	double	start_time,
			end_time,
			runtime,
			s,
			gflops;

	int		whoAmI,
			numOfPro,
			i,
			n;				// vector length

	double	*x,				
			*y;

	MPI_Status stat;

	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);
	MPI_Comm_size(MPI_COMM_WORLD, &numOfPro);

	for( n = 1000 ; n <= 10000 ; n += 1000)
	{
		x = new double [n];
		y = new double [n];

		// processor 0 inizialised the vectors
		if(whoAmI == 0)
		{
			for(i=0;i<n;i++)
			{
				x[i] = rand()%50;
				y[i] = rand()%50;
			}

			start_time = get_time();
			/*	I think the start mesurement must before
				the broadcast because sending is part of the
				algorithem
			*/
		}

		MPI_Bcast(x, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(y, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

		/*	calculating the start and end values
			would be simpler if we just take an even
			number of processors
			the odd resolution is commented out
		*/
		int part = n / numOfPro;
		// int residual = n % numOfPro;
		// int bias = (whoAmI+1 - (numOfPro - residual));
		int start = whoAmI*part ;// + bias;
		int end = start + part;
		
		/*if(bias != 0 )
			end++;*/

		s = inner_product(x,y,start,end);
		double s2;

		for(i=0; i<(int)log(numOfPro)/log(2); i++)
		{
			int k = 1<<i;/*Bit shift to left, i.e. sequence 1,2,4,...*/
			int j = whoAmI^k; /* exclusive OR */
			MPI_Send(&s, 1, MPI_DOUBLE, j, 0, MPI_COMM_WORLD);
			MPI_Recv(&s2, 1, MPI_DOUBLE,j, 0, MPI_COMM_WORLD, &stat);
			s += s2;
		}

		if(whoAmI == 0)
		{
			end_time = get_time();
			runtime = end_time - start_time;
			gflops = n*n / runtime / 1000000000;

			printf("GFLOPS: %f, Runtime: %f \n",gflops,runtime);
		}
		delete[] x;
		delete[] y;
	}

	MPI_Finalize();
	return 0;
}
