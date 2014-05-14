#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include <mpi.h>

int main(int argc, char* argv[])
{
	int whoAmI;
	int worldSize;

	signed char response;
	signed char upDownReq;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	int counter = 0;
	MPI_Status stat;

	//seed random numbers
	srand(time(NULL));
	
	if(whoAmI == 0)
	{
		char 	usedForks = 0;
		int		left = 0, 
				right= 0;

		printf("Kellner (%d) meldet sich an!\n", whoAmI);
		fflush(stdout);
		
		while(counter < worldSize - 1)
		{
			MPI_Recv(&upDownReq, sizeof(signed char), MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
			
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
				usedForks &= ~(1 << upDownReq); //delete bit for this process
			}
			else if (stat.MPI_TAG == 2)
			{
				//This philosopher is finished
				counter++;
			}
			else
			{
				printf("Ungültige Kellneranfrage!\n");
				break;
			}
		}
		printf("Kellner geht nach Hause\n");
		fflush(stdout);
	}
	else
	{
		upDownReq = whoAmI;
		printf("Philosoph %d geht an den Tisch\n", whoAmI);
		fflush(stdout);
		//each Process has to eat 10 times
		while (counter < 10)
		{
			char hungry = rand() % 2;
			while (hungry != 1)
			{
				//do intense thinking
#ifndef WIN32
				sleep(rand() % 2);
#else
				_sleep(rand() % 2);
#endif
				hungry = rand() % 2;
			}

			while (hungry)
			{
				MPI_Send(&upDownReq, sizeof(signed char), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
				MPI_Recv(&response, sizeof(signed char), MPI_CHAR, 0, 1, MPI_COMM_WORLD, &stat);

				if (response == 'W')
				{
					printf("Ich darf nichts essen (%d)\n", whoAmI);
					fflush(stdout);

					// wait a little time (do some thinking)
#ifndef WIN32
					sleep(rand() % 2);
#else
					_sleep(rand() % 2);
#endif
				}
				else if (response == 'E')
				{
					printf("Endlich darf ich (%d) essen... Jam Jam Jam\n", whoAmI);
					fflush(stdout);
					// eat a little time
#ifndef WIN32
					sleep(rand() % 2);
#else
					_sleep(rand() % 2);
#endif

					//Put down fork
					MPI_Send(&upDownReq, sizeof(signed char), MPI_CHAR, 0, 1, MPI_COMM_WORLD);
					printf("Ich (%d) denke jetzt lieber noch ein wenig nach\n", whoAmI);
					fflush(stdout);

					hungry = 0;
					counter++;
				}
			}
		}
		MPI_Send(&upDownReq, sizeof(signed char), MPI_CHAR, 0, 2, MPI_COMM_WORLD);
		printf("Philosoph (%d) meldet sich ab!\n", whoAmI);
		fflush(stdout);
	}

	MPI_Finalize();

	return 0;
}