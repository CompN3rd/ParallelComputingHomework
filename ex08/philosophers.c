#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("Usage: ./philosophers [number of philosophers]\n");
		exit(1);
	}

	int numForks = (int)atoi(argv[1]);

	#pragma omp parallel
	{

	}
}