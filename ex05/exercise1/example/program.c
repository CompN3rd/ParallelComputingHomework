#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

/* ### Problem Specifications ### */

#define M               100    // Number of discrete points in Y-direction
#define N               100    // Number of discrete points in X-direction
#define MAX_ITERATIONS  1000   // Max. number of iterations to compute

/* ### External Interfaces ### */

extern int saveBMP(const char* pathToFile, unsigned char *pixeldata, 
                   unsigned int width, unsigned int height, int invertY);

extern unsigned int jacobi_solve(const double *A,
                          const double *b,
                          double *x,
                          const unsigned int n,
                          const unsigned int maxIterations,
                          const double tolerance);

extern void setupProblem(double *A, double *b, unsigned int m, unsigned int n);

extern void printMatrixToFile(const char *pathToFile, const double *x,
                       unsigned int m, unsigned int n);

extern void visualizeMap(const double *map, unsigned char /*retain*/ **pixels,
                  unsigned int n);

//extern void resetTime();
//extern double getTime();

/* ### Implementation ### */

int main(int argc, char **argv)
{
	//Initialize MPI
	MPI_Init(NULL, NULL);

	int whoAmI;
	MPI_Comm_rank(MPI_COMM_WORLD, &whoAmI);

    // Set up problem
    printf("Setting up problem...\n");
	fflush(stdout);
    double *A = (double*)malloc(sizeof(double)*(M*N)*(M*N));
    double *b = (double*)malloc(sizeof(double)*(M*N));
    double *x = (double*)malloc(sizeof(double)*(M*N));
    
    setupProblem(A, b, M, N);
    memset(x, (char)0, sizeof(double)*M*N);
    
	if (0 == whoAmI)
	{
		printf("ok\n");
		fflush(stdout);
	}

    // [Debug] Uncomment next line of code for printing a matrix/vector to a file.
    // Attention: Only do this for small matrices (i.e. M,N <= 8)!
    //printMatrixToFile("A.dat", A, M, N);
    
	if(0 == whoAmI)
	{
		printf("Solving...\n");
		fflush(stdout);
		//resetTime();
	}
    unsigned int iteration;
    for (iteration = 0; iteration < MAX_ITERATIONS; )
    {
		//visualize first, to see initial solution:
		if(0 == whoAmI)
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
        unsigned int iterationsDone = jacobi_solve(A, b, x, M*N, 10, 0.001);
        if (iterationsDone < 10) break;
        else iteration += iterationsDone;
    }
	if(0 == whoAmI)
	{
		//double timeNeededForSolving = getTime();
		//printf("End of computation!\nTime needed for solving: %fs\n", timeNeededForSolving);
	}
    
    free(A);
    free(b);
    free(x);

	MPI_Finalize();
   
    return 0;
}
