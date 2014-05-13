#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char** artgv)
{
	int resultLen;
	int whoAmI;
	int worldSize;
	int counter = 0;
	MPI_Status stat;

	MPI_Init(NULL, NULL);
    /*	MPI							C
		MPI_CHAR 			-> signed char
		MPI_UNSIGNED_CHAR	-> unsigned char
		From slides 2 page 32
		I think if we use MPI_CHAR we should use signed char in c
	*/
	signed char response;
	signed int upDownReq;

	//seed random numbers
	srand(time(NULL));
	
	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	if(whoAmI == 0)
	{
		char 	usedForks = 0;
		int		left = 0, 
				right= 0;
		
		while(counter < 100)
		{
			MPI_Recv(&upDownReq, sizeof(signed int), MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
			
			// set neighbours
			if(upDownReq == 1)
			{
				left = 5;
				right = 2;
			}else if(upDownReq == 5)
			{
				left = 4;
				right = 1;
			}else
			{
				left = upDownReq - 1;
				right = upDownReq + 1;
			}
			
			//check if forks are put up or put down
			if (stat.MPI_TAG == 0)
			{
				// Bit wise: is fork available?
				if (usedForks & (1 << left) || usedForks & (1 << right))
				{
					// forks are in use -> send 'W' for wait
					response = 'W';
					MPI_Send(&response, sizeof(signed char), MPI_CHAR, upDownReq, 1, MPI_COMM_WORLD);
				}
				else
				{
					// forks not in use -> send 'E' for Eat! 
					response = 'E';
					MPI_Send(&response, sizeof(signed char), MPI_CHAR, upDownReq, 1, MPI_COMM_WORLD);
					usedForks |= (1 << upDownReq); // set bit at pos upDownReq
				}
			}
			else if (stat.MPI_TAG == 1)
			{
				//are both forks really held by this philosopher
				if (usedForks & (1 << left) && usedForks & (1 << right))
				{
					usedForks &= ~(1 << upDownReq); //delete bit for this process
				}
				else
				{
					printf("Du (%d) hattest doch gar nicht beide Gabeln!", upDownReq);
				}
			}
			else
			{
				printf("Ungültige Kellneranfrage!");
				break;
			}

			counter++;
		}
		printf("Kellner geht nach Hause");
	}
	else
	{
		upDownReq = whoAmI;
		//each Process has to eat 10 times
		while (counter < 10)
		{
			char hungry = rand() % 2;
			while (hungry != 1)
			{
				//do intense thinking
				sleep(rand() % 100);
				hungry = rand() % 2;
			}

			while (hungry)
			{
				MPI_Send(&upDownReq, sizeof(signed int), MPI_INT, 0, 0, MPI_COMM_WORLD);
				MPI_Recv(&response, sizeof(signed char), MPI_CHAR, 0, 1, MPI_COMM_WORLD, &stat);

				if (response == 'W')
				{
					printf("Ich darf nichts essen (%d)", whoAmI);

					// wait a little time (do some thinking)
					sleep(rand() % 100);
				}
				else if (response == 'E')
				{
					printf("Endlich darf ich (%d) essen... Jam Jam Jam", whoAmI);
					// eat a little time
					sleep(rand() % 100);

					MPI_Send(&upDownReq, sizeof(signed int), MPI_INT, 0, 1, MPI_COMM_WORLD);

					hungry = 0;
					counter++;
				}
			}
		}
		printf("Philosoph (%d) meldet sich ab!", whoAmI);
	}

	MPI_Finalize();
}
