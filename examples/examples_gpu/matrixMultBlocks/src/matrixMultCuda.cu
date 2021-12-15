#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

#define SEED 6834723

#ifndef BLOCKSIZE
#define BLOCKSIZE 32
#endif
#define GRIDSIZE (SIZE + BLOCKSIZE - 1) / BLOCKSIZE

	Timer tKernel;

//#define CUDA_CHECK() { cudaError_t error; if ((error = cudaGetLastError())!=cudaSuccess){ fprintf(stdout,"%s: %s\n", cudaGetErrorName(error),cudaGetErrorString(error));exit(EXIT_FAILURE); } }
#define CUDA_CHECK()	;

/* Matriz con los datos. */
typedef struct {
	int width;
	int height;
	int stride; 
	float* elements;
} Matrix;

// Get a matrix element
__device__ float GetElement(const Matrix A, int row, int col)
{
	return A.elements[row * A.stride + col];
}

// Set a matrix element
__device__ void SetElement(Matrix A, int row, int col,
		float value)
{
	A.elements[row * A.stride + col] = value;
}

// Get the BLOCKSIZExBLOCKSIZE sub-matrix Asub of A that is
// located col sub-matrices to the right and row sub-matrices down
// from the upper-left corner of A
__device__ Matrix GetSubMatrix(Matrix A, int row, int col) 
{
	Matrix Asub;
	Asub.width    = BLOCKSIZE;
	Asub.height   = BLOCKSIZE;
	Asub.stride   = A.stride;
	Asub.elements = &A.elements[A.stride * BLOCKSIZE * row
		+ BLOCKSIZE * col];
	return Asub;
}


// Matrix multiplication kernel called by MatMul()
__global__ void MatMulKernel(const Matrix A, const Matrix B, Matrix C)
{
	// Block row and column
	int blockRow = blockIdx.y;
	int blockCol = blockIdx.x;

	// Each thread block computes one sub-matrix Csub of C
	Matrix Csub = GetSubMatrix(C, blockRow, blockCol);

	// Each thread computes one element of Csub
	// by accumulating results into Cvalue
	float Cvalue = 0;

	// Thread row and column within Csub
	int row = threadIdx.y;
	int col = threadIdx.x;

	// Loop over all the sub-matrices of A and B that are
	// required to compute Csub
	// Multiply each pair of sub-matrices together
	// and accumulate the results
	for (int m = 0; m < (A.width / BLOCKSIZE); ++m) {

		// Get sub-matrix Asub of A
		Matrix Asub = GetSubMatrix(A, blockRow, m);

		// Get sub-matrix Bsub of B
		Matrix Bsub = GetSubMatrix(B, m, blockCol);

		// Shared memory used to store Asub and Bsub respectively
		__shared__ float As[BLOCKSIZE][BLOCKSIZE];
		__shared__ float Bs[BLOCKSIZE][BLOCKSIZE];

		// Load Asub and Bsub from device memory to shared memory
		// Each thread loads one element of each sub-matrix
		As[row][col] = GetElement(Asub, row, col);
		Bs[row][col] = GetElement(Bsub, row, col);

		// Synchronize to make sure the sub-matrices are loaded
		// before starting the computation
		__syncthreads();

#pragma unroll
		// Multiply Asub and Bsub together
		for (int e = 0; e < BLOCKSIZE; ++e)
			Cvalue += As[row][e] * Bs[e][col];

		// Synchronize to make sure that the preceding
		// computation is done before loading two new
		// sub-matrices of A and B in the next iteration
		__syncthreads();
	}

	// Write Csub to device memory
	// Each thread writes one element
	SetElement(Csub, row, col, Cvalue);
}

// Matrix multiplication - Host code
// Matrix dimensions are assumed to be multiples of BLOCKSIZE
void MatMul(const Matrix A, const Matrix B, Matrix C)
{
	// Load A and B to device memory
	Matrix d_A;
	d_A.width = d_A.stride = A.width; d_A.height = A.height;
	size_t size = A.width * A.height * sizeof(float);
	cudaMalloc(&d_A.elements, size);
	CUDA_CHECK();
	cudaMemcpy(d_A.elements, A.elements, size,
			cudaMemcpyHostToDevice);
	CUDA_CHECK();
	Matrix d_B;
	d_B.width = d_B.stride = B.width; d_B.height = B.height;
	size = B.width * B.height * sizeof(float);
	cudaMalloc(&d_B.elements, size);
	CUDA_CHECK();
	cudaMemcpy(d_B.elements, B.elements, size,
			cudaMemcpyHostToDevice);
	CUDA_CHECK();

	// Allocate C in device memory
	Matrix d_C;
	d_C.width = d_C.stride = C.width; d_C.height = C.height;
	size = C.width * C.height * sizeof(float);
	cudaMalloc(&d_C.elements, size);
	CUDA_CHECK();

	// Invoke kernel
	dim3 dimBlock(BLOCKSIZE, BLOCKSIZE);
	dim3 dimGrid(B.width / dimBlock.x, A.height / dimBlock.y);

	cudaDeviceSynchronize();
	TimerStart(tKernel);
	MatMulKernel<<<dimGrid, dimBlock>>>(d_A, d_B, d_C);
	CUDA_CHECK();
	cudaDeviceSynchronize();
	TimerStop(tKernel);

	// Read C from device memory
	cudaMemcpy(C.elements, d_C.elements, size,
			cudaMemcpyDeviceToHost);
	CUDA_CHECK();

	// Free device memory
	cudaFree(d_A.elements);
	cudaFree(d_B.elements);
	cudaFree(d_C.elements);
	CUDA_CHECK();
}

int main(int argc, char *argv[]) {

	if ( argc!=2 ) {
		fprintf(stderr, "Usage: %s <matrixSize>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int SIZE = atoi( argv[1] );
	size_t MATRIX_SIZE = sizeof(float)*SIZE*SIZE;


	Matrix a, b, c;
	Timer tTotal;

	a.width = a.height = SIZE;
	b.width = b.height = SIZE;
	c.width = c.height = SIZE;

	a.elements = (float*)malloc(MATRIX_SIZE);
	b.elements = (float*)malloc(MATRIX_SIZE);
	c.elements = (float*)malloc(MATRIX_SIZE);

	srand48( SEED );
	TimerCreate(tTotal);
	TimerCreate(tKernel);

	/* INICIALIZA LAS MATRICES */
#ifdef NDEBUG
	for (int i=0; i < SIZE; i++){
		for (int j=0; j < SIZE; j++) {
			a.elements[i * SIZE + j] = (float)drand48();
			b.elements[i * SIZE + j] = (float)drand48();
			c.elements[i * SIZE + j] = 0;
		}
	}
#else
	for (int i=0; i < SIZE; i++){
		for (int j=0; j < SIZE; j++) {
			a.elements[i * SIZE + j] = 18.345;
			b.elements[i * SIZE + j] = 1.14;
			c.elements[i * SIZE + j] = 0;
		}
	}
#endif

	cudaDeviceSynchronize();
	TimerStart(tTotal);

	MatMul(a,b,c);

	cudaDeviceSynchronize();
	TimerStop(tTotal);

#ifdef NDEBUG
	printf("SIZE %d\n", SIZE);
	printf("CLOCK Main %.8lf\n", TimerGetTime(tTotal) );
	printf("CLOCK Kernels %.8lf\n", TimerGetTime(tKernel) );
#else
	printf("Ejecutado en modo depuracion.\n");
	printf("Tamaño de la matriz: %d x %d\n",SIZE,SIZE);
	printf("Tiempo total: %.8lf\n",TimerGetTime(tTotal));
	printf("Tiempo kernel: %.8lf\n",TimerGetTime(tKernel));
	for (int i=0; i<SIZE; i++)
		for (int j=0; j<SIZE; j++) {
			if ( fabs(c.elements[i * SIZE + j] - 18.345 * 1.14 * SIZE) > 0.001 )
			
				fprintf(stderr, "ERROR: Resultado (%d,%d) = %lf\n", i, j,
					 c.elements[i * SIZE + j] );
		}
#endif


	free(a.elements);
	free(b.elements);
	free(c.elements);
	TimerDestroy(tTotal);
	TimerDestroy(tKernel);
	return 0;
}
