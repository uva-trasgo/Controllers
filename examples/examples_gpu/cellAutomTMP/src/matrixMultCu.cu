// System includes
#include <stdio.h>
#include <assert.h>
#include "timer.h"

int GPU=0;

/**
 * Matrix multiplication (CUDA Kernel) on the device: C = A * B
 * wA is A's width and wB is B's width
 */
 
__global__ void copy(float* dst, float* src, int size0, int size1) {
	int row = blockDim.y * blockIdx.y + threadIdx.y;
	int col = blockDim.x * blockIdx.x + threadIdx.x;
	if (row < size0 && col < size1)
		dst[row * size1 + col] = src[row * size1 + col];
}
 
 
 __global__ void
matrixMulCUDA(float *C, float *A, float *B, int wA, int wB)
{
#define BLOCK_SIZE  32
if (blockIdx.x*BLOCK_SIZE+threadIdx.x <wA && blockIdx.y*BLOCK_SIZE+threadIdx.y <wA)
{
    // Block index
    int bx = blockIdx.x;
    int by = blockIdx.y;

    // Thread index
    int tx = threadIdx.x;
    int ty = threadIdx.y;

    // Index of the first sub-matrix of A processed by the block
    int aBegin = wA * BLOCK_SIZE * by;

    // Index of the last sub-matrix of A processed by the block
    int aEnd   = aBegin + wA - 1;

    // Step size used to iterate through the sub-matrices of A
    int aStep  = BLOCK_SIZE;

    // Index of the first sub-matrix of B processed by the block
    int bBegin = BLOCK_SIZE * bx;

    // Step size used to iterate through the sub-matrices of B
    int bStep  = BLOCK_SIZE * wB;

    // Csub is used to store the element of the block sub-matrix
    // that is computed by the thread
    float Csub = 0;

    // Loop over all the sub-matrices of A and B
    // required to compute the block sub-matrix
    for (int a = aBegin, b = bBegin;
         a <= aEnd;
         a += aStep, b += bStep)
    {

        // Declaration of the shared memory array As used to
        // store the sub-matrix of A
        __shared__ float As[BLOCK_SIZE][BLOCK_SIZE];

        // Declaration of the shared memory array Bs used to
        // store the sub-matrix of B
        __shared__ float Bs[BLOCK_SIZE][BLOCK_SIZE];

        // Load the matrices from device memory
        // to shared memory; each thread loads
        // one element of each matrix
        As[ty][tx] = A[a + wA * ty + tx];
        Bs[ty][tx] = B[b + wB * ty + tx];

        // Synchronize to make sure the matrices are loaded
        __syncthreads();

        // Multiply the two matrices together;
        // each thread computes one element
        // of the block sub-matrix
#pragma unroll

        for (int k = 0; k < BLOCK_SIZE; ++k)
        {
            Csub += As[ty][k] * Bs[k][tx];
        }

        // Synchronize to make sure that the preceding
        // computation is done before loading two new
        // sub-matrices of A and B in the next iteration
        __syncthreads();
    }

    // Write the block sub-matrix to device memory;
    // each thread writes one element
    int c = wB * BLOCK_SIZE * by + BLOCK_SIZE * bx;    
    // C[c + wB * ty + tx] = C[c + wB * ty + tx]+Csub;
	}
}


/**
 * Program main
 */
int main(int argc, char **argv)
{

    // ARTURO: Simpler interface for square matrices (only one parameter)
    if ( argc<4 ) {
	fprintf(stderr, "Usage: %s <matrixSize> <matrixSize> <GPU>\n\n", argv[0]);
	exit(EXIT_FAILURE);
    }
    int SIZE = atoi( argv[1] );
    SIZE = atoi( argv[2] );
    GPU = atoi( argv[3] );
	int MATRIX_SIZE = sizeof(float)*SIZE*SIZE;
    // By default, we use device 0, otherwise we override the device ID based on what is provided at the command line

    cudaSetDevice(GPU);  
    
    cudaError_t error;
    cudaDeviceProp deviceProp;

    Timer tTotal, tTotalA,tTotalC;
	Timer tComp;
	Timer tComm;
	TimerCreate(tTotal); TimerCreate(tTotalA); TimerCreate(tTotalC);
	TimerCreate(tComp);
	TimerCreate(tComm);

  
    // Use a larger block size for Fermi and above
    int block_size =  32;

    dim3 dimsA;
    dim3 dimsB;


    // ARTURO: Simpler interface for square matrices (only one parameter)
    dimsA.x = dimsA.y = dimsB.x = dimsB.y = SIZE;

    dim3 threads(block_size, block_size);
    dim3 grid(dimsB.x / threads.x, dimsA.y / threads.y);
	
    printf("MatrixA(%d,%d), MatrixB(%d,%d)\n", dimsA.x, dimsA.y, dimsB.x, dimsB.y);

	/*Variables*/
	float *A, *B, *B2, *C;
	A = (float*)malloc(MATRIX_SIZE);
	B = (float*)malloc(MATRIX_SIZE);
	B2 = (float*)malloc(MATRIX_SIZE);
	C = (float*)malloc(MATRIX_SIZE);

	for (int i = 0; i < SIZE*SIZE; i++)
		{A[i] = 1.0; B[i] = 2.0; B2[i] = 3.0; C[i]=0.0;}

	/*Device variables*/
	float *d_A, *d_B, *d_B2, *d_C;
	cudaMalloc(&d_A, MATRIX_SIZE);
	cudaMalloc(&d_B, MATRIX_SIZE);
	cudaMalloc(&d_B2, MATRIX_SIZE);
	cudaMalloc(&d_C, MATRIX_SIZE);

TimerStart(tTotal); TimerStart(tComm); 
	/*Copia al sdevice*/
	cudaMemcpy(d_A, A, MATRIX_SIZE, cudaMemcpyHostToDevice);
	cudaMemcpy(d_B, B, MATRIX_SIZE, cudaMemcpyHostToDevice);
	cudaMemcpy(d_B2, B2, MATRIX_SIZE, cudaMemcpyHostToDevice);
	cudaMemcpy(d_C, C, MATRIX_SIZE, cudaMemcpyHostToDevice);
	cudaDeviceSynchronize();
TimerStop(tComm); 

TimerStart(tComp);
	matrixMulCUDA<<<grid, threads>>>(d_C, d_A, d_B, SIZE, SIZE);
	copy<<<grid, threads>>>(d_A, d_C, SIZE, SIZE);
	matrixMulCUDA<<<grid, threads>>>(d_C, d_A, d_B2, SIZE, SIZE);
	cudaDeviceSynchronize();
TimerStop(tComp); 

TimerContinue(tComm);
	cudaMemcpy(C, d_C, MATRIX_SIZE, cudaMemcpyDeviceToHost);
	cudaDeviceSynchronize();
TimerStop(tTotal);TimerStop(tComm);	

		printf("\nVERSION SINCRONA************\n" );
        printf("SIZE %d\n", SIZE);
        printf("Clock Main %.8lf <--\n", TimerGetTime(tTotal));
        printf("Clock Comm %.8lf\n", TimerGetTime(tComm));
        printf("Clock Comp %.8lf\n", TimerGetTime(tComp));
        printf("Resultado %lf\n",C[0]);

	free(A); free(B); free(B2); free(C);
	cudaFree(d_A); cudaFree(d_B); cudaFree(d_B2); cudaFree(d_C);   
	
	/*Version asincrona*/												///////////////////////////////////

	/*Variables*/
	float *AA, *BA, *B2A, *CA;
	cudaMallocHost ( &AA, MATRIX_SIZE ) ;
	cudaMallocHost ( &BA, MATRIX_SIZE ) ;
	cudaMallocHost ( &B2A, MATRIX_SIZE ) ;
	cudaMallocHost ( &CA, MATRIX_SIZE ) ;

	/*Device variables*/
	float *d_AA, *d_BA, *d_B2A, *d_CA;
	cudaMalloc(&d_AA, MATRIX_SIZE);
	cudaMalloc(&d_BA, MATRIX_SIZE);
	cudaMalloc(&d_B2A, MATRIX_SIZE);
	cudaMalloc(&d_CA, MATRIX_SIZE);
	
	cudaStream_t stream1, stream2;
	cudaStreamCreate ( &stream1) ; cudaStreamCreate ( &stream2) ; 

	for (int i = 0; i < SIZE*SIZE; i++)
		{AA[i] = 1.0; BA[i] = 2.0; B2A[i] = 3.0; CA[i]=0.0;}

TimerStart(tTotalA);
	/*Copia al device*/
	cudaMemcpyAsync(d_AA, AA, MATRIX_SIZE, cudaMemcpyHostToDevice, stream1);
	cudaMemcpyAsync(d_BA, BA, MATRIX_SIZE, cudaMemcpyHostToDevice, stream1);
	cudaMemcpyAsync(d_CA, CA, MATRIX_SIZE, cudaMemcpyHostToDevice, stream1);

	matrixMulCUDA<<<grid, threads, 0, stream1>>>(d_CA, d_AA, d_BA, SIZE, SIZE);
	copy<<<grid, threads, 0, stream1>>>(d_AA, d_CA, SIZE, SIZE);
	
	cudaMemcpyAsync(d_B2A, B2A, MATRIX_SIZE, cudaMemcpyHostToDevice, stream2);
	
	cudaDeviceSynchronize();
	
	matrixMulCUDA<<<grid, threads, 0, stream2>>>(d_CA, d_AA, d_B2A, SIZE, SIZE);

	cudaMemcpyAsync(CA, d_CA, MATRIX_SIZE, cudaMemcpyDeviceToHost, stream2);
	cudaDeviceSynchronize();
TimerStop(tTotalA);	

		printf("\nVERSION A_SINCRONA************\n" );
        printf("SIZE %d\n", SIZE);
        printf("Clock Main %.8lf <--\n", TimerGetTime(tTotalA));
        printf("Resultado %lf\n",CA[0]);
		printf("\nPorcentaje de mejora (%d x %d) %.3lf%\n", SIZE, SIZE, (100.0 - (TimerGetTime(tTotalA)*100.0)/TimerGetTime(tTotal)));
		
	cudaStreamDestroy(stream1);
	cudaStreamDestroy(stream2);
	cudaFree (AA); cudaFree (BA); cudaFree (B2A); cudaFree (CA);
	cudaFree(d_AA); cudaFree(d_BA); cudaFree(d_B2A); cudaFree(d_CA);   

	/*Version CPU*/												///////////////////////////////////

	/*Variables*/
	float *AC, *BC, *B2C, *CC;
	AC = (float*)malloc(MATRIX_SIZE);
	BC = (float*)malloc(MATRIX_SIZE);
	B2C = (float*)malloc(MATRIX_SIZE);
	CC = (float*)malloc(MATRIX_SIZE);

	for (int i = 0; i < SIZE*SIZE; i++)
		{AC[i] = 1.0; BC[i] = 2.0; B2C[i] = 3.0; CC[i]=0.0;}

TimerStart(tTotalC);
	float parcial=0.0;
	for(int i=0; i<SIZE;i++){
		for(int j=0; j<SIZE;j++){
			for(int k=0; k<SIZE;k++)
				{parcial = parcial +AC[i*SIZE + k]*BC[k*SIZE+j];}
			CC[i*SIZE+j]=CC[i*SIZE+j]+parcial;
			parcial=0.0;}}

	for(int i=0; i<SIZE;i++)
		for(int j=0; j<SIZE;j++)
			for(int k=0; k<SIZE;k++)
				AC[i*SIZE+j] =  CC[i*SIZE+j];
				
	for(int i=0; i<SIZE;i++){
		for(int j=0; j<SIZE;j++){
			for(int k=0; k<SIZE;k++)
				{parcial = parcial +AC[i*SIZE + k]*B2C[k*SIZE+j];}
			CC[i*SIZE+j]=CC[i*SIZE+j]+parcial;
			parcial=0.0;}}

TimerStop(tTotalC);

		printf("\nVERSION CPU************\n" );
        printf("SIZE %d\n", SIZE);
		printf("Clock Main %.8lf <--\n", TimerGetTime(tTotalC));
		printf("Resultado %lf\n",CC[0]);
		printf("\nPorcentaje de mejora (%d x %d) %.3lf%\n", SIZE, SIZE, (100.0 - (TimerGetTime(tTotalC)*100.0)/TimerGetTime(tTotalA)));
	free(AC); free(BC); free(B2C); free(CC);

    return (0);
}
