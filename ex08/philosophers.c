#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, const char** argv)
{
	int numForks;
	int i;
	int eatCount;
	int* forkUsed;
	omp_lock_t forksAccess;

	omp_init_lock(&forksAccess);

	if (argc < 2)
	{
		printf("Usage: ./philosophers [number of philosophers]\n");
		exit(1);
	}

	//get the number of philosophers
	numForks = atoi(argv[1]);
	forkUsed = (int*)malloc(numForks * sizeof(int));
	for(i = 0; i < numForks; i++)
		forkUsed[i] = 0;

	//set the number of threads
	omp_set_dynamic(0);
	omp_set_num_threads(numForks);

	//do the work
	#pragma omp parallel private(eatCount)
	{
		eatCount = 0;
		while(eatCount < 100)
		{
			printf("Philosopher %i is thinking\n", omp_get_thread_num());

			//lock the whole forkUsed array:
			printf("Philosopher %i is trying to gain his forks\n", omp_get_thread_num());

			omp_set_lock(&forksAccess);
			//look if both forks are available
			if(!forkUsed[omp_get_thread_num()] && !forkUsed[(omp_get_thread_num() + 1) % omp_get_num_threads()])
			{
				forkUsed[omp_get_thread_num()] = 1;
				forkUsed[(omp_get_thread_num() + 1) % omp_get_num_threads()] = 1;


				printf("Philosopher %i is taking up his forks\n", omp_get_thread_num());

				//let others have access too
				omp_unset_lock(&forksAccess);
			}
			else
			{
				printf("Philosopher %i has no right to take up his forks\n", omp_get_thread_num());
				omp_unset_lock(&forksAccess);
				//try again
				continue;
			}

			printf("Philosopher %i is eating (for the %i-th time)\n", omp_get_thread_num(), ++eatCount);

			//lay down forks
			omp_set_lock(&forksAccess);
			forkUsed[omp_get_thread_num()] = 0;
			forkUsed[(omp_get_thread_num() + 1) % omp_get_num_threads()] = 0;

			printf("Philosopher %i is laying down his forks\n", omp_get_thread_num());

			omp_unset_lock(&forksAccess);
		}

		//all finished
		printf("Philosopher %i is going home\n", omp_get_thread_num());
	}

	free(forkUsed);
}