#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

/* ### Problem Specifications ### */

#define M               1024    // Number of discrete points in Y-direction
#define N               1024    // Number of discrete points in X-direction
#define MAX_ITERATIONS  10000   // Max. number of iterations to compute

/* ### External Interfaces ### */

extern int saveBMP(const char* pathToFile, unsigned char *pixeldata, 
		unsigned int width, unsigned int height, int invertY);

extern void visualizeMap(const double *map, unsigned char /*retain*/ **pixels,
		unsigned int n);

//extern void resetTime();
//extern double getTime();

/* ### Implementation ### */

void solve(double *x, const double *s,
		unsigned int m, unsigned int n,
		unsigned int iterations)
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Status stat;

	//The number of Lines per processor
	unsigned int numberOfLines;

	//Compute the subdivision of the grid
	//Do the division horizontally in number of lines:
	numberOfLines = M / size;

	unsigned int yStartIndex = rank * numberOfLines;

	//Allocate memory for halo nodes 2 extra lines
	double *topHalo, *bottomHalo;
	topHalo = (double*) malloc(sizeof(double) * N);
	bottomHalo = (double*) malloc(sizeof(double) * N);

	memset(topHalo, 0, sizeof(double) * N);
	memset(bottomHalo, 0, sizeof(double) * N);

	//Fill appropriate halos
	//top, bottom halo
	unsigned int haloIndex;
	for (haloIndex = 0; haloIndex < N; haloIndex++)
	{
		//fill bottom halo
		if (rank < size - 1)
			topHalo[haloIndex] = x[(yStartIndex + numberOfLines) * N + haloIndex];

		//fill top halo
		if (0 < rank)
			bottomHalo[haloIndex] = x[(yStartIndex - 1) * N + haloIndex];
	}

	int i, j, it;
	double *x_new = (double*)malloc(sizeof(double)*M*N);
	for (it = 0; it < iterations; it++)
	{
		//for (i = 0; i < M; i++)
		for (i = yStartIndex; i < yStartIndex + numberOfLines; i++)
		{
			//for (j = 0; j < N; j++)
			for (j = 0; j < N; j++)
			{
				double x_top   = (i < yStartIndex + numberOfLines - 1 ? x[(i+1)*N+j] : topHalo[j]);
				double x_down  = (i > yStartIndex ? x[(i-1)*N+j] : bottomHalo[j]);
				double x_left  = (j > 0 ? x[i*N+(j-1)] : 0);
				double x_right = (j < N-1 ? x[i*N+(j+1)] : 0);
				x_new[i*N+j] = (s[i*N+j]+x_top+x_down+x_left+x_right)/4;
			}
		}
		//copy x_new to correct section of x
		//memcpy(x, x_new, M*N*sizeof(double));
		memcpy(x + yStartIndex * N, x_new + yStartIndex * N, numberOfLines * N * sizeof(double));

		//exchange the correct halos
		//send bottom line, receive top halo:
		if (rank < size - 1)
			MPI_Recv(topHalo, N, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &stat);

		if (rank > 0)
			MPI_Send(x + yStartIndex * N, N, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);

		//send top line, receive bottom halo:
		if (rank > 0)
			MPI_Recv(bottomHalo, N, MPI_DOUBLE, rank - 1, 1, MPI_COMM_WORLD, &stat);

		if (rank < size - 1)
			MPI_Send(x + (yStartIndex + numberOfLines - 1) * N, N, MPI_DOUBLE, rank + 1, 1, MPI_COMM_WORLD);
	}

	free(x_new);
	free(bottomHalo);
	free(topHalo);
}

int main(int argc, char **argv)
{
	MPI_Init(NULL, NULL);

	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Set up problem
	printf("Setting up problem...\n");
	fflush(stdout);

	double *x = (double*)malloc(sizeof(double)*(M*N));
	double *s = (double*)malloc(sizeof(double)*(M*N));
	double *sendbuf = (double*)malloc(sizeof(double) * (M*N / size));

	memset(x, (char)0, sizeof(double)*M*N);
	memset(s, (char)0, sizeof(double)*M*N);

	int i, j;
	for (i = M/4*N; i < M/2*N; i += N)
		for (j = N/4; j < N/2; j++) s[i+j] = 100;

	if (0 == rank)
	{
		printf("ok\n");
		printf("Solving...\n");
		fflush(stdout);
		//resetTime();
	}

	unsigned int iteration;
	for (iteration = 0; iteration < MAX_ITERATIONS; )
	{
		//visualize before solving for initial solution
		if (0 == rank)
		{
			printf("iterations: %d\n", iteration);
			fflush(stdout);

			// visualize
			unsigned char *pixels;
			visualizeMap(x, &pixels, M*N);

			// save bitmap
			printf("Saving bitmap...");
			fflush(stdout);
			char filename[64];
			sprintf(filename, "images/heatmap%d.bmp", iteration);
			if (!saveBMP(filename, pixels, M, N, 0))
			{
				printf("fail!\n");
				fflush(stdout);
				return 1;
			}
			else
			{
				printf("ok\n");
				fflush(stdout);
			}
			free(pixels);
		}

		// solve
		solve(x, s, M, N, 100);
		iteration += 100;

		// Gather x Buffer on processor 0
		memcpy(sendbuf, x + (rank* M / size) * N, M / size * N * sizeof(double));
		MPI_Allgather(sendbuf, M / size * N, MPI_DOUBLE, x, M / size * N, MPI_DOUBLE, MPI_COMM_WORLD);
	}
	if (0 == rank)
	{
		//double timeNeededForSolving = getTime();
		//printf("End of computation!\nTime needed for solving: %fs\n", timeNeededForSolving);
	}

	free(x);
	free(s);
	free(sendbuf);

	MPI_Finalize();

	return 0;
}
