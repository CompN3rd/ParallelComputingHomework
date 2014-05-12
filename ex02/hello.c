#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** artgv)
{
	char proc_name[MPI_MAX_PROCESSOR_NAME];
	int resultLen;
	int whoAmI;
	int worldSize;
	int counter = 1;
	MPI_Status stat;

	MPI_Init(NULL, NULL);

	char* x = (char*)malloc(1024 * sizeof(char));

	MPI_Get_processor_name(proc_name, &resultLen);
	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	if(whoAmI == 0)
	{

		printf("Hello world from processor %s (pid %d), rank %d out of %d!\n", proc_name, getpid(), whoAmI, worldSize);

		while(counter < worldSize)
		{
			MPI_Recv(x, 1024, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &stat);
			printf("%s", x);
			counter++;
		}

	}
	else
	{

		sprintf(x, "Hello world from processor %s (pid %d), rank %d out of %d!\n", proc_name, getpid(), whoAmI, worldSize);
		MPI_Send(x, strlen(x), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	}
	
	free(x);
	MPI_Finalize();
}
