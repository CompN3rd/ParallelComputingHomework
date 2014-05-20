#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
	int x = -1;
	int world_rank;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	if(world_rank == 0)
	{
		x = 0;
		MPI_Bcast(&x,1, MPI_INT, 0, MPI_COMM_WORLD);
	}else
	{
		MPI_Bcast(&x,1, MPI_INT, 0, MPI_COMM_WORLD);
		printf("Bla bla %d \n", x);
	}
	MPI_Finalize();
	return 0;
}
