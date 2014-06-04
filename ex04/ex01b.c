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

			
			/*	I think the start mesurement must before
				the broadcast because sending is part of the
				algorithem
			*/
			// start_time = get_time(); 
		}

		/*
			this Part is sending the initialized vetor to all others
			Iam not sure wich is faster, brodcast the whole vector or send the small vector 
		*/
		MPI_Bcast(x, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(y, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

		start_time = get_time(); 
		
		/*	
			calculating the start and end values
		*/
		int part = n / numOfPro;
		int start = whoAmI*part ;
		int end = start + part;

		int rest = n % numOfPro;
		if(whoAmI+1 > numOfPro - rest)
		{
			start += whoAmI - numOfPro + rest;
			end++;
		}
		double s = inner_product(x,y,start,end);
		
		/*
			Now we have to send the local inner product
		*/
		std::list<int> plist;
		for(i=0; i < numOfPro; i++)
		{
			plist.push_back(i);
		}

		std::list<int>::iterator iter = plist.begin();
		std::list<int>::iterator iterT;

		double s2;
		
		for(i = 0 ;  i <   log(numOfPro)/log(2); i++)
		{
			bool odd = false;
			for(iter = plist.begin(); iter != plist.end(); iter++)
			{	
				if(iter != plist.begin() && !odd)
						plist.erase(std::prev(iter));
				if(!odd)
				{
					odd = true;
					if(*iter == whoAmI)
					{
						if(std::next(iter) != plist.end())
						{
							//printf("%d empfaengt von %d Iteration %d \n",whoAmI,*(std::next(iter)),i);
							MPI_Recv(&s2, 1, MPI_DOUBLE, *(std::next(iter)), 0, MPI_COMM_WORLD, &stat);
							s += s2;
						}
					}				 
				}else
				{
					odd = false;
					if(*iter == whoAmI)
					{
						//printf("%d sendet zu %d Iteration %d \n",whoAmI, *std::prev(iter),i);
						MPI_Send(&s, 1, MPI_DOUBLE, *std::prev(iter), 0, MPI_COMM_WORLD);
					}
				}
			}

			if(plist.size()+1 % 2 == 0)
			{
				plist.pop_back();
			}
		//	MPI_Barrier(MPI_COMM_WORLD); // reach next lvl of the tree
		}

		if(whoAmI == 0)
		{
			end_time = get_time();
			runtime = end_time - start_time;
			gflops = n*n / runtime / 1000000000;

			printf("GFLOPS: %f, Runtime: %f \n",gflops,runtime);
			
			/*
			This algorithem only makes sence if n is very large.
			The reason for that is the overhead of sending an resiving Data
			*/
			// Test Case

			double vergleich;
			for(vergleich=0.,i=0;i<n;i++) 
			{
				vergleich += x[i] * y[i];
			}
			
			// printf("vergleich %f  s = %f \n",vergleich,s);
			
		}
		delete[] x;
		delete[] y;
	}

	MPI_Finalize();
	return 0;
}
