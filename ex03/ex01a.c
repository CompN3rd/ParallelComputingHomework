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
  MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);
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
      			char* x = (char*)malloc(size * 1024 * sizeof(char));
      			start_time = get_time();
			MPI_Bcast(&x, size, MPI_CHAR, 0, MPI_COMM_WORLD);
			end_time = get_time();
      			runtime = end_time - start_time; 
      			printf("%i kb,%f s,%f kbs\n", size, runtime, (size/runtime));
      			free(x);
  		}else
		{ 
	 		char* x = (char*)malloc(size * 1024 * sizeof(char));
			MPI_Bcast(&x, size, MPI_CHAR, 0, MPI_COMM_WORLD);
			free(x);
  		}
 	}	
  	MPI_Finalize();
  	return 0;
}
