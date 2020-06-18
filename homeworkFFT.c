#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <complex.h>

int N;
int P;
double PI;
double *x;
typedef double complex cplx;
cplx *out;
cplx *buf;
int paralelizare = 1;

FILE * file =  NULL;
FILE *fileOut = NULL;

void _fft(cplx buf[], cplx out[], int n, int step)
{
	if (step < n) {
		_fft(out, buf, n, step * 2);
		_fft(out + step, buf + step, n, step * 2);
 
		for (int i = 0; i < n; i += 2 * step) {
			cplx t = cexp(-I * PI * i / n) * out[i + step];
			buf[i / 2]     = out[i] + t;
			buf[(i + n)/2] = out[i] - t;
		}
	}
}

//functia de paralelizare
void* threadFunction1(void *var)
{
	int thread_id = *(int*)var;

	if(P == 1){ //cazul cu 1 thread 
		_fft(buf, out, N, 1);
	}

	if(P == 2){// cazul cu 2 thread-uri
		_fft(out + thread_id, buf + thread_id, N, 2);
	}

	if(P == 4){// cazul cu 4 thread-uri
		int step = 2;
		if(thread_id == 0){
			_fft(buf, out, N, 4);
		}else if(thread_id == 1){
			_fft(buf + step, out + step, N, 4);
		}else if(thread_id == 2){
			_fft(buf + 1, out + 1, N, 4);
		}else if(thread_id == 3){
			_fft(buf + step + 1, out + step + 1, N, 4);
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
	PI = atan2(1, 1) * 4;

	// citirea 
	if( fscanf(file, "%d", &N)!= 1){
		goto end;
	}

	out = (cplx*) malloc(N * sizeof(cplx));
	x = malloc(N * sizeof(double));
	buf = (cplx*) malloc(N * sizeof(cplx));

	for(i = 0; i < N; i++){
		if(fscanf(file, "%lf", &x[i]) != 1)
			break;
		else{
			buf[i] = (cplx) x[i];
		}
	}

	fprintf(fileOut, "%d\n", N);

	for (int i = 0; i < N; i++){
		out[i] = buf[i];
	}

	//paralelizare
	for(i = 0;i < P; i++)
		thread_id[i] = i;

	for(i = 0; i < P; i++){
		pthread_create(&(tid[i]), NULL, threadFunction1, &(thread_id[i]));
	}

	for(i = 0; i < P; i++) {
		pthread_join(tid[i], NULL);
	}
	

	//constructie buf dupa paralelizarea pe ramuri 
	//in functie de numarul de thread-uri

	int step;

	if(P == 2){
		step = 1;
		for (int i = 0; i < N; i += 2 * step ) {
			cplx t = cexp(-I * PI * i / N) * out[i + step];				
			buf[i / 2]     = out[i] + t;
			buf[(i + N)/2] = out[i] - t;
		}
	}

	if(P == 4){
		step = 2;
		for (int i = 0; i < N; i += 2 * step) {
			cplx t = cexp(-I * PI * i / N) * buf[i + step];				
			out[i / 2]     = buf[i] + t;
			out[(i + N)/2] = buf[i] - t;
		}

		for (int i = 0; i < N; i += 2 * step) {
			cplx t = cexp(-I * PI *i/ N) * buf[i + step + 1];				
			out[1 + i / 2]  = buf[i+1] + t;
			out[1 + (i + N)/2] = buf[i+1] - t;
		}
		step = 1;

		for (int i = 0; i < N; i += 2 * step) {
			cplx t = cexp(-I * PI * i / N) * out[i + step];				
			buf[i / 2]     = out[i] + t;
			buf[(i + N)/2] = out[i] - t;
		}

	}

	//sciere in fisier 

	for(i = 0; i < N; i++)
		fprintf(fileOut, "%lf %lf\n", creal(buf[i]), cimag(buf[i]));


end:
	return 0;
}
