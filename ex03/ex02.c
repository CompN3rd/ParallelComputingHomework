#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
	char proc_name[MPI_MAX_PROCESSOR_NAME];
	int resultLen;
	int whoAmI;
	int worldSize;
	int counter = 1;
	MPI_Status stat;
	MPI_Comm comms[4];

	MPI_Init(NULL, NULL);

	char* x = (char*)malloc(1024 * sizeof(char));

	//Make this sufficiently large
	char* names = (char*)malloc(10 * 1024 * sizeof(char));

	MPI_Get_processor_name(proc_name, &resultLen);
	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	// splitting COMM_WORLD in four Groups
	MPI_Comm_split(MPI_COMM_WORLD, whoAmI % 4, 1, &comms[whoAmI % 4]);
	MPI_Barrier(MPI_COMM_WORLD);  // synchronize to ensure all Processors are assigned to one communicator/Group

	int groupRank = -1;
	int groupSize = -1;
	int group = whoAmI % 4;

	MPI_Comm_rank(comms[group], &groupRank);
	MPI_Comm_size(comms[group], &groupSize);

	//Print name information in send buffer
	sprintf(x, "Comm %d: Name: %s Rank: %d \n", group, proc_name, groupRank);
	//Print name information in output buffer
	sprintf(names, "Comm %d: Name: %s Rank: %d \n", group, proc_name, groupRank);

	if (groupRank != groupSize - 1)
	{
		//Send your name to the processor with the highest number
		MPI_Send(x, strlen(x), MPI_CHAR, groupSize - 1, 0, comms[group]);

		//Participate in group Broadcast
		MPI_Bcast(names, 10 * 1024, MPI_CHAR, groupSize - 1, comms[group]);
	}
	else
	{
		//Receive the names from all processors
		for (counter = 0; counter < groupSize - 1; counter++)
		{
			MPI_Recv(x, 1024, MPI_CHAR, MPI_ANY_SOURCE, 0, comms[group], &stat);
			strcat(names, x);
		}

		//Broadcast the names within this group
		MPI_Bcast(names, 10 * 1024, MPI_CHAR, groupSize - 1, comms[group]);
	}

	//Print the names:
	printf("Process of rank %d from group %d says:\n", groupRank, group);
	printf("%s", names);
	fflush(stdout);

	MPI_Comm_free(&comms[group]);


	free(x);
	free(names);
	MPI_Finalize();
}
