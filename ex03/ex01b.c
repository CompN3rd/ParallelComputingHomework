#include <sys/time.h> 
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
	int worldSize;
	int counter = 1;
	
  	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);
 	
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
	printf("I am %d\n", whoAmI);

  	MPI_Status stat;

  	double start_time;
  	double end_time;
  	float runtime;
	int size;
	
	for( size=1000; size < 1000000 ; size +=1000)
	{
		if(whoAmI == 0)
		{
 //			int size = 1000;
      			char* x = (char*)malloc(size * 1024 * sizeof(char));
      			start_time = get_time();
			for(;counter < worldSize; counter++)
			{	
				MPI_Send(x, size * 1024, MPI_CHAR, counter, 0, MPI_COMM_WORLD);
			}
 //     		MPI_Recv(x, 0, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &stat);
//			MPI_Bcast(&x,1, MPI_INT, 0, MPI_COMM_WORLD);
			end_time = get_time();
      			runtime = end_time - start_time; 
      			printf("%i kb,%f s,%f kbs\n", size, runtime, (size/runtime));
      			free(x);
  		}else
		{ 
	 		char* x = (char*)malloc(size * 1024 * sizeof(char));
    			MPI_Recv(x, size*1024, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &stat);
   // 			MPI_Send(x, 0, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
//			MPI_Bcast(&x,1, MPI_INT, 0, MPI_COMM_WORLD);
			free(x);
  		}
 	}	
	
	MPI_Finalize();
  	return 0;
}
