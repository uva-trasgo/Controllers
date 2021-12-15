// System includes
#include <stdio.h>
#include <assert.h>
#include "timer.h"

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 32
#endif

// #ifndef DEBUG
// #define DEBUG
// #endif // DEBUG

int GPU = 0;

#ifdef DEBUG
#define CUDA_CHECK()                                                                                                           \
	{                                                                                                                          \
		cudaError_t error;                                                                                                     \
		if ((error = cudaGetLastError()) != cudaSuccess)                                                                       \
		{                                                                                                                      \
			fprintf(stdout, "%s::%d ERROR: %s: %s\n", __FILE__, __LINE__, cudaGetErrorName(error), cudaGetErrorString(error)); \
			exit(EXIT_FAILURE);                                                                                                \
		}                                                                                                                      \
	}
#else
#define CUDA_CHECK()
#endif

/**
 * Matrix multiplication (CUDA Kernel) on the device: C = A * B
 * wA is A's width and wB is B's width
 */

__global__ void copy(float *dst, float *src, int size0, int size1)
{
	int row = blockDim.y * blockIdx.y + threadIdx.y;
	int col = blockDim.x * blockIdx.x + threadIdx.x;
	if (row < size0 && col < size1)
		dst[row * size1 + col] = src[row * size1 + col];
}

/* 
 * function name: gpu_square_matrix_mult
 * 
 * source: https://github.com/lzhengchun/matrix-cuda/blob/master/matrix_cuda.cu
 */
__global__ void matrixMulCUDA(float *d_c, float *d_a, float *d_b, int n)
{
	__shared__ float tile_a[BLOCK_SIZE][BLOCK_SIZE];
	__shared__ float tile_b[BLOCK_SIZE][BLOCK_SIZE];

	int row = blockIdx.y * BLOCK_SIZE + threadIdx.y;
	int col = blockIdx.x * BLOCK_SIZE + threadIdx.x;
	float tmp = 0.0;
	int idx;

	for (int sub = 0; sub < gridDim.x; ++sub)
	{
		idx = row * n + sub * BLOCK_SIZE + threadIdx.x;
		if (idx >= n * n)
		{
			// n may not divisible by BLOCK_SIZE
			tile_a[threadIdx.y][threadIdx.x] = 0;
		}
		else
		{
			tile_a[threadIdx.y][threadIdx.x] = d_a[idx];
		}

		idx = (sub * BLOCK_SIZE + threadIdx.y) * n + col;
		if (idx >= n * n)
		{
			tile_b[threadIdx.y][threadIdx.x] = 0;
		}
		else
		{
			tile_b[threadIdx.y][threadIdx.x] = d_b[idx];
		}
		__syncthreads();

		for (int k = 0; k < BLOCK_SIZE; ++k)
		{
			tmp += tile_a[threadIdx.y][k] * tile_b[k][threadIdx.x];
		}
		__syncthreads();
	}
	if (row < n && col < n)
	{
		d_c[row * n + col] = tmp;
	}
}

/**
 * Program main
 */
int main(int argc, char **argv)
{

	// ARTURO: Simpler interface for square matrices (only one parameter)
	if (argc != 4)
	{
		fprintf(stderr, "Usage: %s <matrixSize> <n_power> <GPU>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int SIZE = atoi(argv[1]);
	int POWER = atoi(argv[2]);
	GPU = atoi(argv[3]);
	int MATRIX_SIZE = sizeof(float) * SIZE * SIZE;

	// By default, we use device 0, otherwise we override the device ID based on what is provided at the command line

	cudaSetDevice(GPU);
	CUDA_CHECK();

	// cudaError_t error;
	cudaDeviceProp deviceProp;

	Timer tTotal, tTotalA, tTotalC;
	Timer tComp;
	Timer tComm;
	TimerCreate(tTotal);
	TimerCreate(tTotalA);
	TimerCreate(tTotalC);
	TimerCreate(tComp);
	TimerCreate(tComm);

	// Use a larger block size for Fermi and above
	int block_size = BLOCK_SIZE;

	dim3 dimsA;
	dim3 dimsB;

	// ARTURO: Simpler interface for square matrices (only one parameter)
	dimsA.x = dimsA.y = dimsB.x = dimsB.y = SIZE;

	dim3 threads(block_size, block_size);
	dim3 grid((dimsB.x + block_size - 1) / threads.x, (dimsA.y + block_size - 1) / threads.y);

	// Copy result from device memory to host memory
	// dim3 threads(BLOCKSIZE_0, BLOCKSIZE_1);
	// dim3 grid(GRIDSIZE_0, GRIDSIZE_1);

	printf("MatrixA(%d,%d), MatrixB(%d,%d)\n", dimsA.x, dimsA.y, dimsB.x, dimsB.y);
	printf("Threads(%d,%d), Grid(%d,%d)\n", threads.x, threads.y, grid.x, grid.y);

	/*Variables*/
	float *A, *C;
	A = (float *)malloc(MATRIX_SIZE);
	C = (float *)malloc(MATRIX_SIZE);

	for (int i = 0; i < SIZE * SIZE; i++)
	{
		A[i] = 2.0;
		C[i] = 0.0;
	}

	/* Start timer */

	/*Device variables*/
	float *d_A, *d_C, *d_Ci;
	cudaMalloc(&d_A, MATRIX_SIZE);
	cudaMalloc(&d_C, MATRIX_SIZE);
	cudaMalloc(&d_Ci, MATRIX_SIZE);
	CUDA_CHECK();

	// TimerStart(tComm);
	/*Copia al device*/
	cudaMemcpy(d_A, A, MATRIX_SIZE, cudaMemcpyHostToDevice);
	cudaMemcpy(d_C, C, MATRIX_SIZE, cudaMemcpyHostToDevice);
	cudaDeviceSynchronize();
	CUDA_CHECK();
	// TimerStop(tComm);

	// TimerStart(tComp);
	copy<<<grid, threads>>>(d_C, d_A, SIZE, SIZE);
	CUDA_CHECK();
	
	copy<<<grid, threads>>>(d_Ci, d_C, SIZE, SIZE);
	matrixMulCUDA<<<grid, threads>>>(d_C, d_Ci, d_A, SIZE);
	cudaMemcpy(C, d_C, MATRIX_SIZE, cudaMemcpyDeviceToHost);

	cudaDeviceSynchronize();
	TimerStart(tTotal);

	// TimerStop(tComp);
	for (int i = 1; i < POWER; i++)
	{
		TimerContinue(tComp);
		copy<<<grid, threads>>>(d_Ci, d_C, SIZE, SIZE);
		matrixMulCUDA<<<grid, threads>>>(d_C, d_Ci, d_A, SIZE);
		// CUDA_CHECK();
		cudaDeviceSynchronize();
		TimerStop(tComp);

		TimerContinue(tComm);
		cudaMemcpy(C, d_C, MATRIX_SIZE, cudaMemcpyDeviceToHost);
		// CUDA_CHECK();
		cudaDeviceSynchronize();
		TimerStop(tComm);
	}
	// cudaMemcpy(C, d_C, MATRIX_SIZE, cudaMemcpyDeviceToHost);
	// CUDA_CHECK();
	cudaDeviceSynchronize();
	// TimerStop(tComm);
	/* Stop timer */
	TimerStop(tTotal);

	cudaFree(d_A);
	cudaFree(d_C);
	cudaFree(d_Ci);
	CUDA_CHECK();
	// cudaDeviceSynchronize();

	printf("\nVERSION SINCRONA************\n");
	printf("SIZE %d\n", SIZE);
	printf("Clock Main %.8lf <--\n", TimerGetTime(tTotal));
	printf("Clock Comm %.8lf\n", TimerGetTime(tComm));
	printf("Clock Comp %.8lf\n", TimerGetTime(tComp));

	// 11. Calculate NORM
	double resultado = 0, suma = 0;
	for (int i = 0; i < SIZE; i++)
	{
		for (int j = 0; j < SIZE; j++)
		{
#ifdef DEBUG
			printf("%lf ", C[i * SIZE + j]);
#endif
			suma += pow(C[i * SIZE + j], 2);
		}
#ifdef DEBUG
		printf("\n");
#endif //DEBUG
	}
	printf("\n ----------------------- NORM ----------------------- \n");
	printf("\n Acumulated sum: %lf", suma);

	resultado = sqrt(suma);
	printf("\n Result: %lf \n", resultado);

	printf("\n ---------------------------------------------------- \n");

	free(A);
	free(C);

	/*Version asincrona*/ ///////////////////////////////////

	/*Variables*/
	float *AA, *CA;
	// AA = (float*) malloc(MATRIX_SIZE);
	// CA = (float*) malloc(MATRIX_SIZE);
	cudaMallocHost(&AA, MATRIX_SIZE);
	cudaMallocHost(&CA, MATRIX_SIZE);
	CUDA_CHECK();

	for (int i = 0; i < SIZE * SIZE; i++)
	{
		AA[i] = 2.0;
		CA[i] = 0.0;
	}

	/*Device variables*/
	float *d_AA, *d_CA, *d_CiA;
	cudaMalloc(&d_AA, MATRIX_SIZE);
	cudaMalloc(&d_CA, MATRIX_SIZE);
	cudaMalloc(&d_CiA, MATRIX_SIZE);
	CUDA_CHECK();

	cudaStream_t stream1, stream2, stream3;
	cudaStreamCreate(&stream1);
	cudaStreamCreate(&stream2);
	cudaStreamCreate(&stream3);
	CUDA_CHECK();

	cudaEvent_t event1, event2, event3;
	cudaEventCreate(&event1);
	cudaEventCreate(&event2);
	cudaEventCreate(&event3);
	CUDA_CHECK();

	/*Copia al device*/
	cudaMemcpyAsync(d_AA, AA, MATRIX_SIZE, cudaMemcpyHostToDevice, stream1);
	cudaEventRecord(event1, stream1);
	CUDA_CHECK();

	cudaStreamWaitEvent(stream3, event1, 0);
	copy<<<grid, threads, 0, stream3>>>(d_CA, d_AA, SIZE, SIZE);
	cudaEventRecord(event3, stream3);
	CUDA_CHECK();
	
	copy<<<grid, threads, 0, stream3>>>(d_CiA, d_CA, SIZE, SIZE);
	matrixMulCUDA<<<grid, threads, 0, stream3>>>(d_CA, d_CiA, d_AA, SIZE);
	cudaEventRecord(event3, stream3);
	// CUDA_CHECK();

	cudaStreamWaitEvent(stream2, event3, 0);
	cudaMemcpyAsync(CA, d_CA, MATRIX_SIZE, cudaMemcpyDeviceToHost, stream2);
	// CUDA_CHECK();

	/* Start timer */
	cudaDeviceSynchronize();
	TimerStart(tTotalA);

	for (int i = 1; i < POWER; i++)
	{
		copy<<<grid, threads, 0, stream3>>>(d_CiA, d_CA, SIZE, SIZE);
		matrixMulCUDA<<<grid, threads, 0, stream3>>>(d_CA, d_CiA, d_AA, SIZE);
		cudaEventRecord(event3, stream3);
		// CUDA_CHECK();

		cudaStreamWaitEvent(stream2, event3, 0);
		cudaMemcpyAsync(CA, d_CA, MATRIX_SIZE, cudaMemcpyDeviceToHost, stream2);
		// CUDA_CHECK();
	}
	// cudaStreamWaitEvent(stream2, event3, 0);
	// cudaMemcpyAsync(CA, d_CA, MATRIX_SIZE, cudaMemcpyDeviceToHost, stream2);
	// CUDA_CHECK();
	// cudaStreamSynchronize(stream2);
	cudaDeviceSynchronize();

	/* Stop timer */
	TimerStop(tTotalA);
	

	cudaEventDestroy(event1);
	cudaEventDestroy(event2);
	cudaEventDestroy(event3);
	cudaStreamDestroy(stream1);
	cudaStreamDestroy(stream2);
	cudaStreamDestroy(stream3);
	cudaFree(d_AA);
	cudaFree(d_CA);
	cudaFree(d_CiA);
	CUDA_CHECK();
	// cudaDeviceSynchronize();

	printf("\nVERSION A_SINCRONA************\n");
	printf("SIZE %d\n", SIZE);
	printf("Clock Main %.8lf <--\n", TimerGetTime(tTotalA));
	printf("\nPorcentaje de mejora (%d x %d) %.3lf%\n", SIZE, SIZE, (100.0 - (TimerGetTime(tTotalA) * 100.0) / TimerGetTime(tTotal)));

	// 11. Calculate NORM
	resultado = 0, suma = 0;
	for (int i = 0; i < SIZE; i++)
	{
		for (int j = 0; j < SIZE; j++)
		{
#ifdef DEBUG
			printf("%lf ", CA[i * SIZE + j]);
#endif
			suma += pow(CA[i * SIZE + j], 2);
		}
#ifdef DEBUG
		printf("\n");
#endif //DEBUG
	}
	printf("\n ----------------------- NORM ----------------------- \n");
	printf("\n Acumulated sum: %lf", suma);

	resultado = sqrt(suma);
	printf("\n Result: %lf \n", resultado);

	printf("\n ---------------------------------------------------- \n");

	cudaFreeHost(AA);
	cudaFreeHost(CA);

	/*Version CPU*/ ///////////////////////////////////

	// /*Variables*/
	// float *AC, *BC, *B2C, *CC;
	// AC = (float *)malloc(MATRIX_SIZE);
	// BC = (float *)malloc(MATRIX_SIZE);
	// B2C = (float *)malloc(MATRIX_SIZE);
	// CC = (float *)malloc(MATRIX_SIZE);

	// for (int i = 0; i < SIZE * SIZE; i++)
	// {
	// 	AC[i] = 1.0;
	// 	BC[i] = 2.0;
	// 	B2C[i] = 3.0;
	// 	CC[i] = 0.0;
	// }

	// TimerStart(tTotalC);
	// float parcial = 0.0;
	// for (int i = 0; i < SIZE; i++)
	// {
	// 	for (int j = 0; j < SIZE; j++)
	// 	{
	// 		for (int k = 0; k < SIZE; k++)
	// 		{
	// 			parcial = parcial + AC[i * SIZE + k] * BC[k * SIZE + j];
	// 		}
	// 		CC[i * SIZE + j] = CC[i * SIZE + j] + parcial;
	// 		parcial = 0.0;
	// 	}
	// }

	// for (int i = 0; i < SIZE; i++)
	// 	for (int j = 0; j < SIZE; j++)
	// 		for (int k = 0; k < SIZE; k++)
	// 			AC[i * SIZE + j] = CC[i * SIZE + j];

	// for (int i = 0; i < SIZE; i++)
	// {
	// 	for (int j = 0; j < SIZE; j++)
	// 	{
	// 		for (int k = 0; k < SIZE; k++)
	// 		{
	// 			parcial = parcial + AC[i * SIZE + k] * B2C[k * SIZE + j];
	// 		}
	// 		CC[i * SIZE + j] = CC[i * SIZE + j] + parcial;
	// 		parcial = 0.0;
	// 	}
	// }

	// TimerStop(tTotalC);

	// printf("\nVERSION CPU************\n");
	// printf("SIZE %d\n", SIZE);
	// printf("Clock Main %.8lf <--\n", TimerGetTime(tTotalC));
	// printf("Resultado %lf\n", CC[0]);
	// printf("\nPorcentaje de mejora (%d x %d) %.3lf%\n", SIZE, SIZE, (100.0 - (TimerGetTime(tTotalC) * 100.0) / TimerGetTime(tTotalA)));
	// free(AC);
	// free(BC);
	// free(B2C);
	// free(CC);

	return (0);
}
