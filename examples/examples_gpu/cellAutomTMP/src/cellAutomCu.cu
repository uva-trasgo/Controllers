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

#ifdef DEBUG
	#define CUDA_CHECK() { cudaError_t error; if ((error = cudaGetLastError())!=cudaSuccess){ fprintf(stdout,"%s: %s\n", cudaGetErrorName(error),cudaGetErrorString(error));exit(EXIT_FAILURE); } }
#else
	#define CUDA_CHECK()
#endif

__global__ void Copy(float *dst, float *src, int size0, int size1)
{
	int row = blockDim.y * blockIdx.y + threadIdx.y;
	int col = blockDim.x * blockIdx.x + threadIdx.x;
	if (row < size0 && col < size1)
		dst[row * size1 + col] = src[row * size1 + col];
}

__global__ void Update(float *dst, float *src, int size0, int size1)
{
	int row = blockDim.y * blockIdx.y + threadIdx.y + 1;
	int col = blockDim.x * blockIdx.x + threadIdx.x + 1;
	//	if (row == 0 || row >= size0-1 || col == 0 || col >= size1-1 ) return;
	//	if (row < size0-1 && col < size1-1)
	if (row >= size0 - 1 || col >= size1 - 1)
		return;
	dst[row * size1 + col] = (src[(row - 1) * size1 + col] +
							  src[(row + 1) * size1 + col] +
							  src[row * size1 + (col - 1)] +
							  src[row * size1 + (col + 1)]) /
							 4;
}

int main(int argc, char *argv[])
{

	if (argc != 4)
	{
		fprintf(stderr, "Usage: %s <matrixSize> <numStages>  <GPU>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int SIZE = atoi(argv[1]);
	int STAGES = atoi(argv[2]);
	int GPU = atoi(argv[3]);
	int MATRIX_SIZE = sizeof(float) * SIZE * SIZE;

	float *mat, *matA;
	Timer tTotal, tTotalA, tTotalC;
	// Timer tComp;
	// Timer tComm;
	TimerCreate(tTotal);
	TimerCreate(tTotalA);
	TimerCreate(tTotalC);
	// TimerCreate(tComp);
	// TimerCreate(tComm);

	mat = (float *)malloc(MATRIX_SIZE);
	cudaMallocHost(&matA, MATRIX_SIZE);

	srand(SEED);

	for (int i = 0; i < SIZE; i++)
		for (int j = 0; j < SIZE; j++)
		{
			mat[i * SIZE + j] = 0.0;
			matA[i * SIZE + j] = 0.0;
		}

	for (int j = 0; j < SIZE; j++)
	{
		mat[j] = 1;
		matA[j] = 1;
		mat[(SIZE - 1) * SIZE + j] = 2;
		matA[(SIZE - 1) * SIZE + j] = 2;
	}
	for (int i = 0; i < SIZE; i++)
	{
		mat[i * SIZE] = 3;
		matA[i * SIZE] = 3;
		mat[i * SIZE + SIZE - 1] = 4;
		matA[i * SIZE + SIZE - 1] = 4;
	}
	cudaSetDevice(GPU);
	cudaDeviceSynchronize();

	// Copy result from device memory to host memory
	dim3 dimBlock(BLOCKSIZE_0, BLOCKSIZE_1);
	dim3 dimGrid(GRIDSIZE_0, GRIDSIZE_1);

	// Start time counter

	float *d_mat, *d_copy;
	cudaMalloc(&d_mat, MATRIX_SIZE);
	cudaMalloc(&d_copy, MATRIX_SIZE);
	CUDA_CHECK();

	cudaMemcpy(d_mat, mat, MATRIX_SIZE, cudaMemcpyHostToDevice);
	CUDA_CHECK();

	cudaDeviceSynchronize();
	TimerStart(tTotal);
	// TimerStart(tComm);
	// TimerStart(tComp);
	// TimerStop(tComp);
	// Copy vectors from host memory to device memory

	// TimerStop(tComm);
	for (int stage = 0; stage < STAGES; stage++)
	{
		// Update copy
		// TimerContinue(tComp);
		Copy<<<dimGrid, dimBlock>>>(d_copy, d_mat, SIZE, SIZE);
		CUDA_CHECK();

		// Compute iteration
		Update<<<dimGrid, dimBlock>>>(d_mat, d_copy, SIZE, SIZE);
		CUDA_CHECK();
		// cudaDeviceSynchronize();
		// TimerStop(tComp);

		// TimerContinue(tComm);
		cudaMemcpy(mat, d_mat, MATRIX_SIZE, cudaMemcpyDeviceToHost);
		// TimerStop(tComm);
		CUDA_CHECK();

	} //for
	// cudaDeviceSynchronize();

	// TimerContinue(tComm);
	// cudaMemcpy(mat, d_mat, MATRIX_SIZE, cudaMemcpyDeviceToHost); // Free device memory
	// TimerStop(tComm);
	cudaDeviceSynchronize();
	TimerStop(tTotal);

	cudaFree(d_mat);
	cudaFree(d_copy);
	CUDA_CHECK();

	printf("\nVERSION SINCRONA************\n");
	printf("SIZE %d %d\n", SIZE, STAGES);
	printf("Clock Main %.8lf <--\n", TimerGetTime(tTotal));
	// printf("Clock Comm %.8lf\n", TimerGetTime(tComm));
	// printf("Clock Comp %.8lf\n", TimerGetTime(tComp));

	// 11. Calculate NORM
	double resultado = 0, suma = 0;
	for (int i = 0; i < SIZE; i++)
	{
		for (int j = 0; j < SIZE; j++)
		{
#ifdef DEBUG
			printf("%lf \n", mat[i * SIZE + j]);
#endif
			suma += pow(mat[i * SIZE + j], 2);
		}
	}
	printf("\n ----------------------- NORM ----------------------- \n");
	printf("\n Acumulated sum: %lf", suma);

	resultado = sqrt(suma);
	printf("\n Result: %lf \n", resultado);

	printf("\n ---------------------------------------------------- \n");

	/*Version asincrona*/ ///////////////////////////////////

	float *d_matA;
	cudaMalloc(&d_matA, MATRIX_SIZE);
	float *d_copyA;
	cudaMalloc(&d_copyA, MATRIX_SIZE);
	CUDA_CHECK();

	cudaStream_t stream1, stream2;
	cudaStreamCreate(&stream1);
	cudaStreamCreate(&stream2);

	cudaEvent_t event1, event2;
	cudaEventCreate(&event1);
	cudaEventCreate(&event2);

	cudaMemcpyAsync(d_matA, matA, MATRIX_SIZE, cudaMemcpyHostToDevice, stream2);
	cudaEventRecord(event2, stream2);
	// cudaStreamSynchronize(stream2);
	cudaDeviceSynchronize();
	TimerStart(tTotalA);

	for (int stage = 0; stage < STAGES; stage++)
	{

		// Update copy
		cudaStreamWaitEvent(stream1, event2, 0);
		Copy<<<dimGrid, dimBlock, 0, stream1>>>(d_copyA, d_matA, SIZE, SIZE);
		CUDA_CHECK();

		// Compute iteration
		Update<<<dimGrid, dimBlock, 0, stream1>>>(d_matA, d_copyA, SIZE, SIZE);
		cudaEventRecord(event1, stream1);
		CUDA_CHECK();

		// Update Host image
		cudaStreamWaitEvent(stream2, event1, 0);
		cudaMemcpyAsync(matA, d_matA, MATRIX_SIZE, cudaMemcpyDeviceToHost, stream2);
		CUDA_CHECK();
		// cudaDeviceSynchronize();
	} //for
	// cudaStreamSynchronize(stream2);
	
	cudaDeviceSynchronize();
	TimerStop(tTotalA);

	cudaFree(d_matA);
	cudaFree(d_copyA);
	cudaStreamDestroy(stream1);
	cudaStreamDestroy(stream2);
	cudaEventDestroy(event1);
	cudaEventDestroy(event2);
	CUDA_CHECK();
	// cudaDeviceSynchronize();

	printf("\nVERSION A_SINCRONA************\n");
	printf("SIZE %d %d\n", SIZE, STAGES);
	printf("Clock Main %.8lf <--\n", TimerGetTime(tTotalA));
	printf("\nPorcentaje de mejora (%d x %d; iter %d) %.3lf%\n", SIZE, SIZE, STAGES, (100.0 - (TimerGetTime(tTotalA) * 100.0) / TimerGetTime(tTotal)));

	// 11. Calculate NORM
	resultado = 0, suma = 0;
	for (int i = 0; i < SIZE; i++)
	{
		for (int j = 0; j < SIZE; j++)
		{
#ifdef DEBUG
			printf("%lf \n", matA[i * SIZE + j]);
#endif
			suma += pow(matA[i * SIZE + j], 2);
		}
	}
	printf("\n ----------------------- NORM ----------------------- \n");
	printf("\n Acumulated sum: %lf", suma);

	resultado = sqrt(suma);
	printf("\n Result: %lf \n", resultado);

	printf("\n ---------------------------------------------------- \n");

	free(mat);
	cudaFree(matA);
	TimerDestroy(tTotal);
	TimerDestroy(tComm);

	/*Version CPU*/ ///////////////////////////////////
	// float *matC, *matC2;
	// matC = (float *)malloc(MATRIX_SIZE);
	// matC2 = (float *)malloc(MATRIX_SIZE);
	// srand(SEED);

	// for (int i = 0; i < SIZE; i++)
	// 	for (int j = 0; j < SIZE; j++)
	// 	{
	// 		matC[i * SIZE + j] = 0.0;
	// 	}

	// for (int j = 0; j < SIZE; j++)
	// {
	// 	matC[j] = 1;
	// 	matC[(SIZE - 1) * SIZE + j] = 2;
	// }
	// for (int i = 0; i < SIZE; i++)
	// {
	// 	matC[i * SIZE] = 3;
	// 	matC[i * SIZE + SIZE - 1] = 4;
	// }
	// TimerStart(tTotalC);
	// for (int stage = 0; stage < STAGES - 1; stage++)
	// {

	// 	for (int j = 0; j < SIZE; j++)
	// 		for (int k = 0; k < SIZE; k++)
	// 			matC2[j * SIZE + k] = matC[j * SIZE + k];

	// 	for (int j = 1; j < SIZE - 1; j++)
	// 		for (int k = 1; k < SIZE - 1; k++)
	// 			matC[j * SIZE + k] = (matC2[(j - 1) * SIZE + (k)] + matC2[(j + 1) * SIZE + (k)] + matC2[(j)*SIZE + (k - 1)] + matC2[(j)*SIZE + (k - 1)]) / 4;
	// } //for
	// TimerStop(tTotalC);

	// printf("\nVERSION CPU************\n");
	// printf("SIZE %d %d\n", SIZE, STAGES);
	// printf("Clock Main %.8lf <--\n", TimerGetTime(tTotalC));
	// printf("\nPorcentaje de mejora (%d x %d; iter %d) %.3lf%\n", SIZE, SIZE, STAGES, (100.0 - (TimerGetTime(tTotalC) * 100.0) / TimerGetTime(tTotalA)));

	// free(matC);
	// free(matC2);

	return 0;
}
