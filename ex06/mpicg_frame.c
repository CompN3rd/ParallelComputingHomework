#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <string.h>

#define CONST_TOL 0.00001
#define MPI_RANK_ROOT 0
#define MAX_ITER 1000

void readFile(char *fileName, int *n, double **value, int **colind, int **rbegin, double **b){
	FILE *fp=fopen(fileName,"r");
	char buf[200];
	int i,j,k,l;
	int p,q;
	double w;
	int m;
	if ((fp=fopen(fileName,"r"))==NULL){
		fprintf(stderr,"Open file error, check filename please.\n");
	}
	fgets(buf,200,fp);
	*n=atoi(buf);
	l=0;
	while(buf[l++]!=' ');
	m=atoi(&buf[l]);
	printf("Matrix size: %d, #Non-zeros: %d\n",*n,m);
	(*value)=(double*)malloc(sizeof(double)*m);
	(*colind)=(int*)malloc(sizeof(int)*m);
	(*rbegin)=(int*)malloc(sizeof(int)*((*n)+1));
	(*b)=(double*)malloc(sizeof(double)*(*n));
	for(i=0;i&lt;(*n);i++){
		(*b)[i]=1.0;
	}
	k=-1;
	for(i=0;i&lt;m;i++){
		fgets(buf,200,fp);
		l=0;p=atoi(&buf[l]);
		while(buf[l++]!=' '); q=atoi(&buf[l]);
		while(buf[l++]!=' '); w=atof(&buf[l]);
		(*value)[i]=w;
		(*colind)[i]=q;
		if(p!=k){
			k=p;
			(*rbegin)[p]=i;
		}
	}
	(*rbegin)[*n]=m;
	fclose(fp);
}

void scatterData(int *n, int *m, double **value, int **colind, int **rbegin, double **b, int rank, int nproc){
	int np;
	int *sendcnts;
	int *displs;
	double *gvalue;
	int *gcolind;
	int *grbegin;
	double *gb;
	int i,j;
	if (rank==MPI_RANK_ROOT){
		sendcnts=(int*)malloc(sizeof(int)*(nproc));
		displs=(int*)malloc(sizeof(int)*(nproc));
		np=(*n)/nproc;
		gvalue=(*value);(*value)=NULL;
		gcolind=(*colind);(*colind)=NULL;
		grbegin=(*rbegin);(*rbegin)=NULL;
		gb=(*b);(*b)=NULL;
		for(i=0;i&lt;nproc;i++){
			sendcnts[i]=0;
			for(j=i*np;j&lt;(i+1)*np;j++){
				sendcnts[i]+=grbegin[j+1]-grbegin[j];
			}
		}
		displs[0]=0;
		for(i=1;i&lt;nproc;i++){
			displs[i]=displs[i-1]+sendcnts[i-1];
		}
	}
	fflush(stdout);
	MPI_Bcast(n,1,MPI_INT,MPI_RANK_ROOT,MPI_COMM_WORLD);
	MPI_Scatter(sendcnts,1,MPI_INT,m,1,MPI_INT,MPI_RANK_ROOT,MPI_COMM_WORLD);
	np=(*n)/nproc;
	printf("Process %d get n=%d, m=%d\n",rank,*n,*m);
	fflush(stdout);
	(*value)=(double*)malloc(sizeof(double)*(*m));
	(*colind)=(int*)malloc(sizeof(int)*(*m));
	(*rbegin)=(int*)malloc(sizeof(int)*(np+1));
	(*b)=(double*)malloc(sizeof(double)*np);
	MPI_Scatterv(gvalue, sendcnts, displs, MPI_DOUBLE, (*value), (*m),MPI_DOUBLE, MPI_RANK_ROOT, MPI_COMM_WORLD);
	MPI_Scatterv(gcolind, sendcnts, displs, MPI_INT, (*colind), (*m),MPI_INT, MPI_RANK_ROOT, MPI_COMM_WORLD);
	MPI_Scatter(grbegin, np, MPI_INT, (*rbegin), np, MPI_INT, MPI_RANK_ROOT, MPI_COMM_WORLD);
	MPI_Scatter(gb, np, MPI_DOUBLE, (*b), np, MPI_DOUBLE, MPI_RANK_ROOT, MPI_COMM_WORLD);
	int offset=(*rbegin)[0];
	for(i=0;i&lt;np;i++){
		(*rbegin)[i]-=offset;
	}
	(*rbegin)[np]=(*m);
	if (rank==MPI_RANK_ROOT){
		free(gvalue);
		free(gcolind);
		free(grbegin);
		free(gb);
		free(sendcnts);
		free(displs);
	}
}

void writeFile(int n,double *answer){
	FILE *fp=fopen("output.dat","w");
	int i;
	for(i=0;i&lt;n;i++){
		fprintf(fp,"%.10f\n",answer[i]);
	}
	fclose(fp);
}

double* cg(int n, double *value, int* colind, int* rbegin, double *b, int rank, int nproc){
	double *answer;
	// Your CODE here. NOTICE: Data is already Scattered. Function IS already Called.

	double *r_old, *r_new, *sendbuf, *z, *x;
	double* d;
	double numerator;
	double denominator;
	double alpha;
	double beta;
	double resNorm;

	//x, r_old, r_new, z, sendbuf reduced size
	x = (double*) malloc(n / nproc * sizeof(double));
	r_old = (double*) malloc(n / nproc * sizeof(double));
	r_new = (double*) malloc(n / nproc * sizeof(double));
	z = (double*) malloc(n / nproc * sizeof(double));
	sendbuf = (double*) malloc(n / nproc * sizeof(double));

	//d, answer full size
	d = (double*) malloc(n * sizeof(double));
	answer = (double*) malloc(n * sizeof(double));

	int row, col;

	//compute Residuum r = b - Ax, start with x = 0
	for(row = 0; row < n / nproc; row++)
	{
		x[row] = 0;

		//residuum very simple :D
		r_old[row] = b[row];
		d[row] = b[row];
	}

	int k;
	for(k = 0; k < (n < MAX_ITER ? n : MAX_ITER); k++)
	{
		//exchange d_k
		memcpy(sendbuf, d, n / nproc * sizeof(double));
		MPI_Allgather(sendbuf, n / nproc, MPI_DOUBLE, d, n / nproc, MPI_DOUBLE, MPI_COMM_WORLD);

		//compute z = A * d_k, and partSum = z^T * z
		numerator = 0;
		denominator = 0;
		for(row = 0; row < n / nproc; row++)
		{
			z[row] = 0;
			for(col = rbegin[row]; col < rbegin[row + 1] - 1; col++)
			{
				z[row] += value[col] * d[colind[col]];
			}
			numerator += z[row] * z[row];
			denominator += d[rank * n / nproc + row] * z[row];
		}

		//compute sum of partial Sums for numerator and denominator
		MPI_Allreduce(&numerator, &numerator, nproc, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
		MPI_Allreduce(&denominator, &denominator, nproc, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

		//alpha_k = (r_k^T * r_k) / (d_k^T * z)
		alpha = numerator / denominator;

		//compute new x_k, r_k
		//compute new numerator (warning! save in denominator)
		denominator = 0;
		for(row = 0; row < n / nproc; row++)
		{
			//x_{k+1} = x_k + a_k * d_k
			x[row] += alpha * d[rank * n / nproc];

			//r_{k+1} = r_k - a_k * z
			r_new[row] = r_old[row] - alpha * z[row];

			//r_{k+1}^T * r_{k+1}
			//warning! save in denominator
			denominator += r_new[row] * r_new[row];
		}

		//compute new numerator (warning! save in denominator)
		MPI_Allreduce(&denominator, &denominator, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

		//compute beta: new denominator is old numerator
		beta = denominator / numerator;

		//compute new d_{k+1}
		for(row = 0; row < n / nproc; row++)
			d[row] = r_new[row] + beta * d[rank * n / nproc + row];

		//compute new resNorm in infinity norm:
		resNorm = 0;
		for(row = 0; row < n / nproc; row++)
		{
			if(fabs(r_new[row]) > resNorm)
				resNorm = fabs(r_new[row]);
		}
		MPI_Allreduce(&resNorm, &resNorm, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

		if(resNorm < CONST_TOL)
			break;

		//copy r_new to r_old
		memcpy(r_old, r_new, n / nproc * sizeof(double));
	}


	//gather all partial x in answer buffer
	MPI_Allgather(x, n / nproc, MPI_DOUBLE, answer, n / nproc, MPI_DOUBLE, MPI_COMM_WORLD);

	free(r_old);
	free(r_new);
	free(d);
	free(z);
	free(x);
	free(sendbuf);
	return answer;
}

int main(int argc, char* argv[]){
	int n;
	int m;
	double *value=NULL;
	int *colind=NULL;
	int *rbegin=NULL;
	double *answer=NULL;
	double *b=NULL;
	int nproc,rank,namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&nproc);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Get_processor_name(processor_name,&namelen);
	printf("Process %d on %s out of %d\n",rank, processor_name, nproc);
	fflush(stdout);
	if (rank==MPI_RANK_ROOT){
		readFile(argv[1],&n,&value,&colind,&rbegin,&b);
		scatterData(&n,&m,&value,&colind,&rbegin,&b,rank,nproc);
	}
	else{
		scatterData(&n,&m,&value,&colind,&rbegin,&b,rank,nproc);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	double tv1,tv2;
	tv1=MPI_Wtime();
	answer=cg(n,value,colind,rbegin,b,rank,nproc);
	tv2=MPI_Wtime();
	if (rank==MPI_RANK_ROOT){
		printf("Process %d takes %.10f seconds\n",rank,tv2-tv1);
	}
	if (value!=NULL) {
		free(value);
	}
	if (colind!=NULL) {
		free(colind);
	}
	if (rbegin!=NULL) {
		free(rbegin);
	}
	if (b!=NULL) {
		free(b);
	}
	if (rank==MPI_RANK_ROOT) {
		writeFile(n,answer);
	}
	if (answer!=NULL) {
		free(answer);
	}
	MPI_Finalize();
	return 1;
}	

