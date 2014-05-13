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
    /*	MPI							C
		MPI_CHAR 			-> signed char
		MPI_UNSIGNED_CHAR	-> unsigned char
		From slides 2 page 32
		I think if we use MPI_CHAR we should use signed char in c
	*/
	signed char* response = (signed char*)malloc(sizeof(signed char));
	signed int* eatReq = (int*)malloc( sizeof(signed int));
	
	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	if(whoAmI == 0)
	{
		char 	byte = 0;
		int		left = 0, 
				right= 0;
		
		while(counter < worldSize)
		{
			MPI_Recv(eatReq, sizeof(signed int), MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &stat);
			
			// set neighbours
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
			
			// Bit wise: is fork available?
			if( byte & (1 << left) || byte & ( 1 << right ))
			{
				// forks are in use -> send 'W' for wait
				response[0] = 'W';
				MPI_Send(response, sizeof(signed char), MPI_CHAR, eatReq[0], 1, MPI_COMM_WORLD);
			}else
			{
				// forks not in use -> send 'E' for Eat! 
				response[0] = 'E';
				MPI_Send(response, sizeof(signed char), MPI_CHAR, eatReq[0], 1, MPI_COMM_WORLD);
				byte |= (1 << eatReq[0]); // set bit at pos eatReq[0]
				//  byte or ...
			}
			
			// ToDo:
			/*
				Receive that the Process has finish eating
				byte &= ~(1 << finish);   delete bit for this process
	
			*/		
			counter++;
		}
	}
	else
	{	
		eatReq[0]		= whoAmI;	
		char hungrig	= 1;
		
		while(hungrig)
		{
			MPI_Send(eatReq, sizeof(signed int), MPI_INT, 0, 0, MPI_COMM_WORLD);
			MPI_Recv(response, sizeof(signed char), MPI_CHAR, 0, 1, MPI_COMM_WORLD, &stat);
		
			if(response[0] == 'W')
			{
				// std::cout << "Ich darf nichts essen" << std::endl;
				printf("Ich darf nichts essen");
				/* ToDo:
					Wartezeit ? 
				*/
			}else if(response[0] == 'E')
			{
				// std::cout << "Endlich darf ich essen" << std::endl;
				printf("Endlich darf ich essen... Jam Jam Jam");
				hungrig = 0;
			}
		}
	}
	
	free(eatReq);
	free(response);
	
	MPI_Finalize();
}
