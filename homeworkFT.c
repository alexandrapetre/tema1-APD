#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <complex.h>

int N;
int P;
double* x;
double complex *xRez;

FILE * file =  NULL;
FILE *fileOut = NULL;

//functia de paralelizare
void* threadFunction(void *var)
{
	int thread_id = *(int*)var;
	int k, n;
	int start, end;

	start = ceil(N / P) * thread_id;
	end =  fmin(ceil(N/P) * (thread_id + 1), N);

	for(k = start; k < end; k++){
		for(n = 0; n < N; n++){
			double angle = 2.0 * M_PI * n * k / N;
			xRez[k] += x[n] * cexp(-angle * I);
		}
	}

	pthread_exit( NULL);
}


int main(int argc, char * argv[]) {

	file = fopen(argv[1] , "r");
	fileOut = fopen(argv[2], "w");
	P = atoi(argv[3]);

	int i;
	pthread_t tid[P];
	int thread_id[P];

	// citirea 
	if( fscanf(file, "%d", &N)!= 1){
		goto end;
	}

	x = malloc(sizeof(double) * N);
	xRez = (double complex *) malloc(N * sizeof(double complex));

	for(i = 0; i < N; i++){
		if(fscanf(file, "%lf", &x[i]) != 1)
			break;
	}

	fprintf(fileOut, "%d\n", N);

	//paralelizare
	for(i = 0;i < P; i++)
		thread_id[i] = i;

	for(i = 0; i < P; i++) {
		pthread_create(&(tid[i]), NULL, threadFunction, &(thread_id[i]));
	}

	for(i = 0; i < P; i++) {
		pthread_join(tid[i], NULL);
	}

	//scriere in fisier
	
	for(i = 0; i < N; i++)
		fprintf(fileOut, "%lf %lf\n", creal(xRez[i]), cimag(xRez[i]));

end:
	return 0;
}
