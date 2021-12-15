#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

#define SEED 6834723


/*
#ifndef BLOCKSIZE
#define BLOCKSIZE 16
#endif
*/
#ifndef BLOCKSIZE_0
#define BLOCKSIZE_0 128
#endif
#ifndef BLOCKSIZE_1
#define BLOCKSIZE_1 2
#endif

#define GRIDSIZE_0 (SIZE + BLOCKSIZE_0 - 1) / BLOCKSIZE_0
#define GRIDSIZE_1 (SIZE + BLOCKSIZE_1 - 1) / BLOCKSIZE_1

//#define CUDA_CHECK() { cudaError_t error; if ((error = cudaGetLastError())!=cudaSuccess){ fprintf(stdout,"%s: %s\n", cudaGetErrorName(error),cudaGetErrorString(error));exit(EXIT_FAILURE); } }
#define CUDA_CHECK()


__global__ void Copy(float* dst, float* src, int size0, int size1) {
	int row = blockDim.y * blockIdx.y + threadIdx.y;
	int col = blockDim.x * blockIdx.x + threadIdx.x;
	if (row < size0 && col < size1)
		dst[row * size1 + col] = src[row * size1 + col];
}

__global__ void Update(float* dst, float* src, int size0, int size1) {
	int row = blockDim.y * blockIdx.y + threadIdx.y + 1;
	int col = blockDim.x * blockIdx.x + threadIdx.x + 1;
//	if (row == 0 || row >= size0-1 || col == 0 || col >= size1-1 ) return;
//	if (row < size0-1 && col < size1-1)
	if (row >= size0-1 || col >= size1-1) return;
	dst[row * size1 + col] = ( 
			src[(row-1) * size1 + col] + 
			src[(row+1) * size1 + col] +
			src[row * size1 + (col-1)] +
			src[row * size1 + (col+1)] ) / 4;
}

int main(int argc, char *argv[]) {

	if ( argc != 4 ) {
		fprintf(stderr, "Usage: %s <matrixSize> <numStages> <GPU>\n", argv[0] );
		exit( EXIT_FAILURE );
	}
	int SIZE = atoi(argv[1]);
	int STAGES = atoi(argv[2]);
	int GPU = atoi(argv[3]);
	int MATRIX_SIZE = sizeof(float)*SIZE*SIZE;

	float* mat;
	Timer tTotal;
	Timer tKernels;

	mat  = (float*)malloc(MATRIX_SIZE);

	srand( SEED );
	TimerCreate(tTotal);
	TimerCreate(tKernels);

	for ( int i=0; i<SIZE; i++ )
		for ( int j=0; j<SIZE; j++ )
			mat[i*SIZE+j] = 0.0;

	for ( int j=0; j<SIZE; j++ ) {
		mat[j] = 1;
		mat[(SIZE-1) * SIZE +j] = 2;
	}
	for ( int i=0; i<SIZE; i++ ) {
		mat[i*SIZE] = 3;
		mat[i*SIZE + SIZE-1] = 4;
	}
        cudaSetDevice(GPU);
        cudaDeviceSynchronize();

	TimerStart(tTotal);

	float* d_mat;
	cudaMalloc(&d_mat, MATRIX_SIZE);
	float* d_copy;
	cudaMalloc(&d_copy, MATRIX_SIZE);
	CUDA_CHECK();

	// Copy vectors from host memory to device memory
	cudaMemcpy(d_mat, mat, MATRIX_SIZE, cudaMemcpyHostToDevice);
	CUDA_CHECK();

	// Copy result from device memory to host memory
	dim3 dimBlock(BLOCKSIZE_0, BLOCKSIZE_1);
	dim3 dimGrid(GRIDSIZE_0, GRIDSIZE_1);

	cudaDeviceSynchronize();	
	TimerStart(tKernels);
	for (int stage=0; stage<STAGES; stage++ ) {
		// Update copy
		Copy<<<dimGrid, dimBlock>>>(d_copy, d_mat, SIZE, SIZE);
		CUDA_CHECK();
		
		// Compute iteration
		Update<<<dimGrid,dimBlock>>>(d_mat, d_copy, SIZE, SIZE);
		CUDA_CHECK();
	}
	cudaDeviceSynchronize();
	TimerStop(tKernels);

	cudaMemcpy(mat, d_mat, MATRIX_SIZE, cudaMemcpyDeviceToHost); // Free device memory
	cudaFree(d_mat);
	cudaFree(d_copy);
	CUDA_CHECK();

	cudaDeviceSynchronize();
	TimerStop(tTotal);

        printf("SIZE %d %d\n", SIZE, STAGES );
        printf("Clock Main %.8lf\n", TimerGetTime(tTotal));

        // 11. Calculate NORM
        double resultado=0,suma=0;
        for (int i=0; i<SIZE; i++){
                for (int j=0; j<SIZE; j++) {
                        #ifdef DEBUG
                                printf("%lf \n", mat[ i * SIZE + j ]);
                        #endif
                         suma += pow(  mat[ i * SIZE + j ] ,2);
        }}
        printf("\n ----------------------- NORM ----------------------- \n");
        printf("\n Acumulated sum: %lf",suma);

        resultado=sqrt( suma );
        printf("\n Result: %lf \n",resultado);

        printf("\n ---------------------------------------------------- \n");

	free(mat);
	TimerDestroy(tTotal);
	TimerDestroy(tKernels);
	return 0;
}
