#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** artgv)
{
	int resultLen;
	int whoAmI;
	int worldSize;
	int counter = 1;
	MPI_Status stat;

	MPI_Init(NULL, NULL);

	char* x = (char*)malloc(1024 * sizeof(char));
    
	signed char* response = (signed char*)malloc(sizeof(signed char));
	signed int* eatReq = (int*)malloc( sizeof(signed int));
	
	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	if(whoAmI == 0)
	{
		char byte = 0;
		// printf("Hello world from processor %s (pid %d), rank %d out of %d!\n", proc_name, getpid(), whoAmI, worldSize);
		// was ist wenn 0 als erste ist 10sec vergehen und kein Recv kommt ?
		// macht doch keinen Sinn 
		while(counter < worldSize)
		{

			MPI_Recv(eatReq, sizeof(signed int), MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &stat);
			
			int left,right;
			
			if(eatReq[0] == 1)
			{
				left = 5;
				right = 2;
			}else if(eatReq[0] == 5)
			{
				left = 4;
				right = 1;
			}else
			{
				left = eatReq[0] - 1;
				right = eatReq[0] + 1;
			}
			
			// Bit Abfrage ob rechts oder links die Gabel weg ist
			if( byte & left || byte & right)
			{
				// nicht essen -> sende warten
				response[0] = 'W';
				MPI_Send(response, sizeof(signed char), MPI_CHAR, eatReq[0], 1, MPI_COMM_WORLD);
			}else
			{
				// essen -> sende Iss
				response[0] = 'I';
				MPI_Send(response, sizeof(signed char), MPI_CHAR, eatReq[0], 1, MPI_COMM_WORLD);
				byte ^ eatReq[0]; // Bit setzen => er isst
			}
			
			counter++;
		}
	}
	else
	{
		
		//sprintf(x, "Hello world from processor %s (pid %d), rank %d out of %d!\n", proc_name, getpid(), whoAmI, worldSize);
		eatReq[0] = whoAmI;
		
		
		char hungrig = 1;
		while(hungrig)
		{
			MPI_Send(eatReq, sizeof(signed int), MPI_INT, 0, 0, MPI_COMM_WORLD);
		
		
			MPI_Recv(response, sizeof(signed char), MPI_CHAR, 0, 1, MPI_COMM_WORLD, &stat);
		
			if(response[0] == 'W')
			{
				// std::cout << "Ich darf nichts essen" << std::endl;
				printf("Ich darf nichts essen");
			}else if(response[0] == 'I')
			{
				// std::cout << "Endlich darf ich essen" << std::endl;
				printf("Endlich darf ich essen... Jam Jam Jam");
				hungrig = 0;
			}
		}
	}
	
	free(x);
	free(eatReq);
	free(response);
	
	
	MPI_Finalize();
}
