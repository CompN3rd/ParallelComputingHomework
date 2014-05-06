#include <time.h>
#include <stdlib.h>
#include <stdio.h>

//Initialize variables
double start_time = 0;
double stop_time  =0;
double end_time = 0;
double sum;
int j;
int i;
int n ;
int s;

//Function to get the current time in seconds
double get_time()
{
  struct timeval time;
  gettimeofday(&time, NULL);
  return (double) time.tv_sec + (time.tv_usec/1000000.0);
}

int main(void) {
  // Array of length n with random numbers
  n = 10;
  double* x = malloc(n * sizeof(double));
  // TO-DO Init with random numbers
  for (i = 0; i < n; i++) {
    x[i] = i;
  }
  // Calculate sum
  s = 1;
  start_time = get_time();
  for (j =0; j < n; j+=s) {
    sum +=x[j];
  }
  // Stop time, print sum and time
  free(x);
  stop_time = get_time();
  end_time = stop_time - start_time;
  printf("Sum: %f \n", sum);
  printf("Computation time: %fs", end_time);
}
