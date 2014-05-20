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
	
	
	MPI_Init(NULL, NULL);

	char* x = (char*)malloc(1024 * sizeof(char));
	char* name = (char*)malloc(4*1024 * sizeof(char));
	
	MPI_Get_processor_name(proc_name, &resultLen);
	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
	
	int newRank = (int)(whoAmI/4);
	int highest = (int)(worldSize/4);
	// splitting COMM_WORLD in four Groups
	
	switch(whoAmI%4)
	{
		case 0:
			if(worldSize % 4 > 0)
				highest++;
			MPI_Comm_split(MPI_COMM_WORLD,0, newRank, &comm0);
		break;
		
		case 1: 
			if(worldSize % 4 > 1)
				highest++;
			MPI_Comm_split(MPI_COMM_WORLD,1, newRank, &comm1); 
		break;
		
		case 2:
			if(worldSize % 4 > 2)
				highest++;
			MPI_Comm_split(MPI_COMM_WORLD,2, newRank, &comm2); 
		break;
		
		case 3:
			if(worldSize % 4 > 3)
				highest++;
			MPI_Comm_split(MPI_COMM_WORLD,3, newRank, &comm3); 	
		break;
	}
	
	MPI_Barrier(MPI_COMM_WORLD);  // synchronize to ensure all Processors are assigned to one communicator/Group
	
	/*
		I don't know if there is a way to link again his group
		or just to check if he is in. I think that would be very nice.
	*/
	int groupRank = -1;
	int groupSize = -1;
	switch(whoAmI%4)
	{
		case 0: 
			MPI_Comm_rank(comm0, &groupRank);
			MPI_Comm_size(comm0, &groupSize);
			
			sprintf(x, "Comm0: Name: %s Rank: %d \n", proc_name,groupRank );
			
			if(groupRank != highest)
			{
				MPI_Send(&groupRank, strlen(x), MPI_CHAR, highest, 0, comm0);
			}else
			{
				for( counter = 1 ; counter < groupSize; counter++)
				{
					MPI_Recv(x, 1024*sizeof(char), MPI_CHAR, MPI_ANY_SOURCE, 0, comm0, &stat);
					strcat(name,x);
				}			
			}
			
			MPI_Bcast(name,4 * 1024 * sizeof(char), MPI_CHAR, highest, comm0);
			
			
			// Slides:  - has to be called by all process belongs to it
			//			- After call comm0 = MPI_COMM_NULL 
			// 			should there be some MPI_Barrier(comm0); ? 
			MPI_Comm_free(&comm0);
		break;
		
		case 1:	
			MPI_Comm_rank(comm1, &groupRank);
			MPI_Comm_size(comm1, &groupSize);
			
			sprintf(x, "Comm0: Name: %s Rank: %d \n", proc_name,groupRank );
			MPI_Send(&groupRank, strlen(x), MPI_CHAR, highest, 0, comm1);
			
			if(groupRank != highest)
			{
				MPI_Send(&groupRank, strlen(x), MPI_CHAR, highest, 0, comm1);
			}else
			{
				for(counter = 1 ; counter < groupSize; counter++)
				{
					MPI_Recv(x, 1024*sizeof(char), MPI_CHAR, MPI_ANY_SOURCE, 0, comm1, &stat);
					strcat(name,x);
				}			
			}
			
			MPI_Bcast(name,4 * 1024 * sizeof(char), MPI_CHAR, highest, comm1);
			MPI_Comm_free(&comm1);
		break;
		
		case 2: 
			MPI_Comm_rank(comm2, &groupRank);
			MPI_Comm_size(comm2, &groupSize);
			
			sprintf(x, "Comm0: Name: %s Rank: %d \n", proc_name,groupRank );
			MPI_Send(&groupRank, strlen(x), MPI_CHAR, highest, 0, comm2);

			if(groupRank != highest)
			{
				MPI_Send(&groupRank, strlen(x), MPI_CHAR, highest, 0, comm2);
			}else
			{
				for( counter = 1 ; counter < groupSize; counter++)
				{
					MPI_Recv(x, 1024*sizeof(char), MPI_CHAR, MPI_ANY_SOURCE, 0, comm2, &stat);
					strcat(name,x);
				}			
			}
			
			MPI_Bcast(name,4 * 1024 * sizeof(char), MPI_CHAR, highest, comm2);
			MPI_Comm_free(&comm2);
		break;
		
		case 3: 
			MPI_Comm_rank(comm3, &groupRank);
			MPI_Comm_size(comm3, &groupSize);
			
			sprintf(x, "Comm0: Name: %s Rank: %d \n", proc_name,groupRank );
			MPI_Send(&groupRank, strlen(x), MPI_CHAR, highest, 0, comm3);
			
			if(groupRank != highest)
			{
				MPI_Send(&groupRank, strlen(x), MPI_CHAR, highest, 0, comm3);
			}else
			{
				for( counter = 1 ; counter < groupSize; counter++)
				{
					MPI_Recv(x, 1024*sizeof(char), MPI_CHAR, MPI_ANY_SOURCE, 0, comm3, &stat);
					strcat(name,x);
				}			
			}
			
			MPI_Bcast(name,4 * 1024 * sizeof(char), MPI_CHAR, highest, comm3);
				
			MPI_Comm_free(&comm3);
		break;
	}
	
	printf(name);
	fflush(stdout);
	
	free(x);
	free(name);
	MPI_Finalize();
}
