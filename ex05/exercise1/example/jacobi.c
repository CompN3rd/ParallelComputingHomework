#include <math.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

/*
 Computes a solution x for the linear equation system Ax=b.
 The solution will be computed iteratively using the Jacobi method.
 
 Parameters:
    [in]     A              - an nxn system matrix (row-wise)
    [in]     b              - right hand side (n-dim vector)
    [in/out] x              - startvector at beginning, solution after computation
    [in]     n              - dimension
    [in]     maxIterations  - max. number of iterations to compute before abort
    [in]     tolerance      - min. change of solution without abort
 
 Returns:
    number of iterations actually computed
 */
unsigned int jacobi_solve(const double *A,
                    const double *b,
                    double *x,
                    const unsigned int n,
                    const unsigned int maxIterations,
                    const double tolerance)
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);

    double *xNew = (double*)malloc(sizeof(double)*n);
    double change;

	//Calculate offsets and starting positions for Allgatherv
	double* sendbuf = xNew + rank * n / size;
	int sendcount = rank < size - 1 ? n / size : n - (size - 1) * n / size;
	double* recvbuf = (double*)malloc(sizeof(double) * n);
	int* recvcount = (int*)malloc(sizeof(int) * size);
	int* displs = (int*)malloc(sizeof(int) * size);
	int processor;
	for(processor = 0; processor < size; processor++)
	{
		recvcount[processor] = n / size;
		displs[processor] = processor * n / size;
	}
	//Overwrite last element in recvcount with correct value
	recvcount[size - 1] = n - (size - 1) * n / size;
    
    int iterations, i, j;
    for (iterations = 0; iterations < maxIterations; iterations++)
    {
        //for (i = 0; i < n; i++)
		for (i = rank * n / size; i < (rank < size - 1 ? (rank + 1) * n / size : n); i++)
        {
            xNew[i] = 0.;
            for (j = 0; j < n; j++)
            {
                if (j != i)
                    xNew[i] += A[i*n+j]*x[j];
            }
            //xNew[i] = (b[i]-xNew[i])/A[i*n+i];
			xNew[i] = (b[i]-xNew[i])/A[i*n+i];
        }

		//Send xNew[appropriateBounds] to all other processors
		MPI_Allgatherv(sendbuf, sendcount, MPI_DOUBLE, recvbuf, recvcount, displs, MPI_DOUBLE, MPI_COMM_WORLD);
		memcpy(xNew, recvbuf, sizeof(double) * n);

        for (i = 0, change = 0.; i < n; i++)
        {
            change += fabs(xNew[i]-x[i]);
            x[i] = xNew[i];
        }
        if (change <= tolerance) break;
    }
    
    free(xNew);
	free(recvbuf);
	free(recvcount);
	free(displs);
    return iterations;
}
