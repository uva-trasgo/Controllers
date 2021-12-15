#include <stdio.h>
#include <hitmap.h>
#include <stdlib.h>
#include "timer.h"

#define SEED 6834723

#ifndef BLOCKSIZE_0
#define BLOCKSIZE_0 128
#endif
#ifndef BLOCKSIZE_1
#define BLOCKSIZE_1 2
#endif

#define GRIDSIZE_0 ((SIZE + BLOCKSIZE_0 - 1) / BLOCKSIZE_0)
#define GRIDSIZE_1 ((SIZE + BLOCKSIZE_1 - 1) / BLOCKSIZE_1)

//#define CUDA_CHECK() { cudaError_t error; if ((error = cudaGetLastError())!=cudaSuccess){ fprintf(stdout,"%s: %s\n", cudaGetErrorName(error),cudaGetErrorString(error));exit(EXIT_FAILURE); } }
#define CUDA_CHECK()

__global__ void MatAdd(float* A, float* B, float* C, int size0, int size1, int NITER ) {
	int i = blockDim.y * blockIdx.y + threadIdx.y;
	int j = blockDim.x * blockIdx.x + threadIdx.x;
	if ( i<size0 && j<size1 )
		for (int iter=0; iter<NITER; iter++)
			C[i*size1+j] = C[i*size1+j] + A[i*size1+j] + B[i*size1+j];
}


HitClock mainClock;
HitClock seqClock;
HitClock commClock;

int main(int argc, char *argv[]) {

	if ( argc<3 ) {
		fprintf(stderr, "Usage: %s <matrixSize> <numIter>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	long long int SIZE = atoi( argv[1] );
	long long int NITER = atoi( argv[2] );
	size_t MATRIX_SIZE = sizeof(float) * SIZE * SIZE;

	float* A;
	float* B;
	float* C;


	A = (float*)malloc(MATRIX_SIZE);
	B = (float*)malloc(MATRIX_SIZE);
	C = (float*)malloc(MATRIX_SIZE);

	if ( A == NULL || B == NULL || C == NULL ) {
		fprintf(stderr,"ERROR: Allocating host memory\n");
		exit ( EXIT_FAILURE );
	}
	

	srand( SEED );
        cudaSetDevice(0);
	cudaDeviceSynchronize();

	/* INICIALIZA LAS MATRICES */
#ifdef NDEBUG
	for (int i=0; i < SIZE; i++){
		for (int j=0; j < SIZE; j++) {
			A[i * SIZE + j] = rand();
			B[i * SIZE + j] = rand();
			C[i * SIZE + j] = 0;
		}
	}
#else
	for (int i=0; i < SIZE; i++){
		for (int j=0; j < SIZE; j++) {
			A[i * SIZE + j] = 0.12;
			B[i * SIZE + j] = 0.2;
			C[i * SIZE + j] = 0;
		}
	}
#endif
	cudaDeviceSynchronize();
	hit_clockReset( mainClock );
	hit_clockReset( seqClock );
	hit_clockReset( commClock );
	hit_clockStart( mainClock );


	cudaDeviceSynchronize();
	hit_clockStart( commClock );
	float* d_A;
	cudaMalloc(&d_A, MATRIX_SIZE);
	//CUDA_CHECK();
	float* d_B;
	cudaMalloc(&d_B, MATRIX_SIZE);
	//CUDA_CHECK();
	float* d_C;
	cudaMalloc(&d_C, MATRIX_SIZE);
	//CUDA_CHECK();

	// Copy vectors from host memory to device memory
	cudaMemcpy(d_A, A, MATRIX_SIZE, cudaMemcpyHostToDevice);
	//CUDA_CHECK();
	cudaMemcpy(d_B, B, MATRIX_SIZE, cudaMemcpyHostToDevice);
	//CUDA_CHECK();
	cudaMemcpy(d_C, C, MATRIX_SIZE, cudaMemcpyHostToDevice);
	//CUDA_CHECK();

	cudaDeviceSynchronize();
	hit_clockStop( commClock );


	// Invoke kernel
	dim3 grid( GRIDSIZE_0, GRIDSIZE_1 );
	dim3 block( BLOCKSIZE_0, BLOCKSIZE_1 );

#ifdef DEGUG
printf("CTRL Params: grid(%d,%d), block(%d,%d)\n", GRIDSIZE_0, GRIDSIZE_1,
						BLOCKSIZE_0, BLOCKSIZE_1 );
#endif

	cudaDeviceSynchronize();
	hit_clockStart( seqClock );
	
	MatAdd<<<grid, block>>>(d_A, d_B, d_C, SIZE, SIZE, NITER);
	
	cudaDeviceSynchronize();
	hit_clockStop( seqClock );
	
	//CUDA_CHECK();

	// Copy result from device memory to host memory
	// C contains the result in host memory

	cudaDeviceSynchronize();
	hit_clockContinue( commClock );
	cudaMemcpy(C, d_C, MATRIX_SIZE, cudaMemcpyDeviceToHost); // Free device memory
	//CUDA_CHECK();
	cudaFree(d_A);
	cudaFree(d_B);
	cudaFree(d_C);
	//CUDA_CHECK();
	cudaDeviceSynchronize();
	hit_clockStop( commClock );

	cudaDeviceSynchronize();
	hit_clockStop( mainClock );

        printf("\nmatrixAdd SIZE: %d %d\n", SIZE, NITER );
        printf("Clock main: %.8lf\n", mainClock.seconds);
        printf("Clock comm: %.8lf\n", commClock.seconds);
        printf("Clock seq: %.8lf\n", seqClock.seconds);

#ifdef DEBUG
	printf("Ejecutado en modo depuracion.\n");
	for (int i=0; i<SIZE; i++) 
		for (int j=0; j<SIZE; j++) 
			if ( fabs(C[i*SIZE+j] - 0.32 * NITER ) > 0.01 )
				fprintf(stderr, "ERROR: Result C[%d,%d] = %lf\n", i, j, C[i*SIZE+j]);
#endif

	free(A);
	free(B);
	free(C);
	return 0;
}
