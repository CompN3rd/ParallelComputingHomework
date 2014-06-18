#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

/* ### Helper Specifications ### */

unsigned int log2( unsigned int x )
{
  unsigned int ans = 0 ;
  while( x>>=1 ) ans++;
  return ans ;
}

/* ### Problem Specifications ### */

#define M               1024    // Number of discrete points in Y-direction
#define N               1024    // Number of discrete points in X-direction
#define MAX_ITERATIONS  10000   // Max. number of iterations to compute

/* ### External Interfaces ### */

extern int saveBMP(const char* pathToFile, unsigned char *pixeldata, 
                   unsigned int width, unsigned int height, int invertY);

extern void visualizeMap(const double *map, unsigned char /*retain*/ **pixels,
                  unsigned int n);

extern void resetTime();
extern double getTime();

/* ### Implementation ### */

void solve(double *x, const double *s,
		unsigned int m, unsigned int n,
		unsigned int iterations)
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int logProc = log2((unsigned int) size);
	unsigned int xSize, ySize;

	//Compute the subdivision of the grid
	if(0 == logProc % 2)
	{
		xSize = (unsigned int)pow(2.0, logProc / 2);
		ySize = (unsigned int)pow(2.0, logProc / 2);
	}
	else
	{
		xSize = (unsigned int)pow(2.0, (logProc + 1) / 2);
		ySize = (unsigned int)pow(2.0, (logProc - 1) / 2);
	}

	//Allocate memory for halo nodes
	unsigned int xIndex = rank % xSize;
	unsigned int yIndex = (rank - xIndex) / xSize;

	double *leftHalo, *rightHalo, *topHalo, *bottomHalo;
	leftHalo = (double*) malloc(sizeof(double) * M / ySize);
	rightHalo = (double*) malloc(sizeof(double) * M / ySize);
	topHalo = (double*) malloc(sizeof(double) * N / xSize);
	bottomHalo = (double*) malloc(sizeof(double) * N / xSize);

	memset(leftHalo, 0, sizeof(double) * M / ySize);
	memset(rightHalo, 0, sizeof(double) * M / ySize);
	memset(topHalo, 0, sizeof(double) * N / xSize);
	memset(bottomHalo, 0, sizeof(double) * N / xSize);

	//Send Buffers for Communication exchange:
	double* horBuf = (double*)malloc(sizeof(double) * M / ySize);
	double* vertBuf = (double*)malloc(sizeof(double) * N / xSize);

	//Fill appropriate halos
	//left, right halo
	int haloIndex;
	for (haloIndex = 0; haloIndex < M / ySize; haloIndex++)
	{
		if (xIndex > 0)
			leftHalo[haloIndex] = x[(yIndex * M / ySize + haloIndex) * N + xIndex * N / xSize - 1];

		if (xIndex < xSize - 1)
			rightHalo[haloIndex] = x[(yIndex * M / ySize + haloIndex) * N + (xIndex + 1) * N / xSize];
	}

	//top, bottom halo
	for (haloIndex = 0; haloIndex < N / xSize; haloIndex++)
	{
		if (yIndex > 0)
			bottomHalo[haloIndex] = x[(yIndex * M / ySize - 1) * N + xIndex * N / xSize + haloIndex];

		if (yIndex < ySize - 1)
			topHalo[haloIndex] = x[(yIndex * M / ySize + 1) * N + xIndex * N / xSize + haloIndex];
	}

    int i, j, it;
    double *x_new = (double*)malloc(sizeof(double)*M*N);
    for (it = 0; it < iterations; it++)
    {
        //for (i = 0; i < M; i++)
		for (i = yIndex * M / ySize; i < (yIndex + 1) * M / ySize; i++)
        {
            //for (j = 0; j < N; j++)
			for (j = xIndex * N / xSize; j < (xIndex + 1) * N / xSize; j++)
            {
                double x_top   = (i < (yIndex + 1) * M / ySize - 1 ? x[(i+1)*N+j] : topHalo[j - xIndex * N / xSize]);
                double x_down  = (i > yIndex * M / ySize   ? x[(i-1)*N+j] : bottomHalo[j - xIndex * N / xSize]);
                double x_left  = (j > xIndex * N / xSize   ? x[i*N+(j-1)] : leftHalo[i - yIndex * M / ySize]);
                double x_right = (j < (xIndex + 1) * N / xSize - 1 ? x[i*N+(j+1)] : rightHalo[i - yIndex * M / ySize]);
                x_new[i*N+j] = (s[i*N+j]+x_top+x_down+x_left+x_right)/4;
            }
        }
        //memcpy(x, x_new, M*N*sizeof(double));

		//exchange the correct halos
		if (xIndex > 0)
		{
			//Send left column and receive left halo
			for (i = 0; i < M / ySize; i++)
			{
				horBuf[i] = x[(yIndex * M / ySize + i) * N + xIndex * N / xSize];
			}
			MPI_Send(horBuf, M / ySize, MPI_DOUBLE, yIndex * xSize + xIndex - 1, 0, MPI_COMM_WORLD);
			
		}
    }

    free(x_new);
	free(leftHalo);
	free(rightHalo);
	free(bottomHalo);
	free(topHalo);
	free(horBuf);
	free(vertBuf);
}

int main(int argc, char **argv)
{
	MPI_Init(NULL, NULL);

    // Set up problem
    printf("Setting up problem...");
    double *x = (double*)malloc(sizeof(double)*(M*N));
    double *s = (double*)malloc(sizeof(double)*(M*N));
    
    memset(x, (char)0, sizeof(double)*M*N);
    memset(s, (char)0, sizeof(double)*M*N);
    
    int i, j;
    for (i = M/4*N; i < M/2*N; i += N)
        for (j = N/4; j < N/2; j++) s[i+j] = 100;

    printf("ok\n");
       
    printf("Solving...\n");
    resetTime();
    unsigned int iteration;
    for (iteration = 0; iteration < MAX_ITERATIONS; )
    {
        // solve
        solve(x, s, M, N, 100);
		iteration += 100;
        printf("iterations: %d\n", iteration);
        
        // visualize
        unsigned char *pixels;
        visualizeMap(x, &pixels, M*N);
        
        // save bitmap
        printf("Saving bitmap...");
        char filename[64];
        sprintf(filename, "images/heatmap%d.bmp", iteration);
        if (!saveBMP(filename, pixels, M, N, 0))
        {
            printf("fail!\n");
            return 1;
        }
        else
        {
            printf("ok\n");
        }
        free(pixels);
    }
    double timeNeededForSolving = getTime();
    printf("End of computation!\nTime needed for solving: %fs\n", timeNeededForSolving);
    
    free(x);
    free(s);

	MPI_Finalize();

    return 0;
}
