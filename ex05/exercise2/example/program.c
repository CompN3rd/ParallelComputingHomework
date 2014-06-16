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

	if(0 == xIndex)
	{
	}
	else if(xIndex == xSize - 1)
	{
	}
	else
	{
	}

	if(0 == yIndex)
	{
	}
	else if(yIndex == ySize - 1)
	{
	}
	else
	{
	}

    int i, j, it;
    double *x_new = (double*)malloc(sizeof(double)*M*N);
    for (it = 0; it < iterations; it++)
    {
        //for (i = 0; i < M; i++)
		for (i = yIndex * M / ySize; i < yIndex * M / ySize + ySize; i++)
        {
            //for (j = 0; j < N; j++)
			for (j = xIndex * N / xSize; j < xIndex * N / xSize + xSize; j++)
            {
                double x_top   = (i < M-1 ? x[(i+1)*N+j] : 0);
                double x_down  = (i > 0   ? x[(i-1)*N+j] : 0);
                double x_left  = (j > 0   ? x[i*N+(j-1)] : 0);
                double x_right = (j < N-1 ? x[i*N+(j+1)] : 0);
                x_new[i*N+j] = (s[i*N+j]+x_top+x_down+x_left+x_right)/4;
            }
        }
        //memcpy(x, x_new, M*N*sizeof(double));

		//exchange
    }

    free(x_new);
	free(leftHalo);
	free(rightHalo);
	free(bottomHalo);
	free(topHalo);
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
