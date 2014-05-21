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
	MPI_Comm comm0, comm1, comm2, comm3;
	// MPI_Comm* commA = (MPI_Comm*)malloc(4 * sizeof(MPI_Comm));
	MPI_Comm commA[4];
	
	MPI_Init(NULL, NULL);

	char* x = (char*)malloc(1024 * sizeof(char));
	char* name = (char*)malloc(4*1024 * sizeof(char));
	
	MPI_Get_processor_name(proc_name, &resultLen);
	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
	
	// splitting COMM_WORLD in four Groups
	
	switch(whoAmI%4)
	{
		case 0:
			MPI_Comm_split(MPI_COMM_WORLD,0,1, &comm0);
		break;
		
		case 1: 
			MPI_Comm_split(MPI_COMM_WORLD,1, 1, &comm1); 
		break;
		
		case 2:
			MPI_Comm_split(MPI_COMM_WORLD,2, 1, &comm2); 
		break;
		
		case 3:
			MPI_Comm_split(MPI_COMM_WORLD,3, 1, &comm3); 	
		break;
	}
	
	MPI_Barrier(MPI_COMM_WORLD);  // synchronize to ensure all Processors are assigned to one communicator/Group
	
	int groupRank = -1;
	int groupSize = -1;
	int group = whoAmI % 4;
	
	MPI_Comm_rank(commA[group], &groupRank);
	MPI_Comm_size(commA[group], &groupSize);
			
	sprintf(x, "Comm%d: Name: %s Rank: %d \n",group ,proc_name, groupRank );
			
	if(groupRank != groupSize -1)
	{
		MPI_Send(&groupRank, strlen(x), MPI_CHAR, groupSize -1, 0, commA[group]);
	}else
	{
		for( counter = 1 ; counter < groupSize; counter++)
		{
			MPI_Recv(x, 1024*sizeof(char), MPI_CHAR, MPI_ANY_SOURCE, 0, commA[group], &stat);
			strcat(name,x);
		}			
	}
			
	MPI_Bcast(name,4 * 1024 * sizeof(char), MPI_CHAR, groupSize -1, comm0);

			// Slides:  - has to be called by all process belongs to it
			//			- After call comm0 = MPI_COMM_NULL 
			// 			should there be some MPI_Barrier(comm0); ? 
	MPI_Comm_free(&commA[group]);	
	

	printf(name);
	fflush(stdout);
	
	free(x);
	free(name);
	MPI_Finalize();
}
