#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "timer.h"
#include "CAL.h"

#define SEED 6834723

#ifndef BLOCKSIZE
#define BLOCKSIZE 32
#endif

#undef CUDA_CHECK
#define CUDA_CHECK() { cudaError_t error; if ((error = cudaGetLastError())!=cudaSuccess){ fprintf(stdout,"%s: %s\n", cudaGetErrorName(error),cudaGetErrorString(error));exit(EXIT_FAILURE); } }

Timer tInner;

hit_tileNewType( float );
hit_ktileNewType( float );

/* A. CPU Kernel charazterization */
CAL_KERNEL_GPU_CHAR_STATIC(MatMult, 2, fixed, square, 32);

/* B. CPU Kernel implementation */
CAL_KERNEL(MatMult, dGPU, KHitTile_float A, KHitTile_float B, KHitTile_float C){
	// Block positions
	int aCol = 0;
	int bRow = 0;
	int bCol = blockIdx.x * BLOCKSIZE;

	// Thread index
	int row = threadIdx.y;
	int col = threadIdx.x;

	// Each thread computes one element of Csub
	// by accumulating results into Cvalue
	float Cvalue = 0;

	
	// Loop over all the sub-matrices of A and B
	// required to compute the block sub-matrix
	for (int iter=0; 
		iter < A.origAcumCard[1] / BLOCKSIZE; 
		iter++, aCol+=BLOCKSIZE, bRow+=BLOCKSIZE )
	{
		// Shared memory used to store Asub and Bsub respectively
		__shared__ float As[BLOCKSIZE][BLOCKSIZE];
		__shared__ float Bs[BLOCKSIZE][BLOCKSIZE];

		// Load the matrices from device memory
		// to shared memory; each thread loads
		// one element of each matrix
		As[row][col] = hit_tileElemAtNoStride( A, 2, threadId.y, aCol + col );
		Bs[row][col] = hit_tileElemAtNoStride( B, 2, bRow + row, bCol + col );

		// Synchronize to make sure the matrices are loaded
		__syncthreads();

		// Multiply the two matrices together;
		// each thread computes one element
		// of the block sub-matrix
		#pragma unroll
		for (int k = 0; k < BLOCKSIZE; ++k)
			Cvalue += As[row][k] * Bs[k][col];

		// Synchronize to make sure that the preceding
		// computation is done before loading two new
		// sub-matrices of A and B in the next iteration
		__syncthreads();
	}

	// Write the block sub-matrix to device memory;
	// each thread writes one element
	hit_tileElemAtNoStride(C, 2, threadId.y, threadId.x) = Cvalue;
}


/* C. Defining kernel prototypes */
CAL_KERNEL_PROTO( MatMult,                                               // Name
                  1, dGPU,	  	                                  // Implementations
                  3, IN, HitTile_float, A, IN, HitTile_float, B, OUT, HitTile_float, C
                );

/*
 * Main program to perform matrix multiplication
 */
int main(int argc, char *argv[]) {
#ifdef DEBUG
        setbuf(stderr, NULL);
        setbuf(stdout, NULL);
#endif

        // 1. Init Controllers library
        //CAL_CntrlInit(2);
        omp_set_nested(1);
        omp_set_num_threads(3);
        #pragma omp parallel 
        {
        #pragma omp single 
         {

	// 2. Taking arguments
	if ( argc!=4 ) {
		fprintf(stderr, "Usage: %s <matrixSize> <matrixSize> <GPU>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int SIZE = atoi( argv[1] );
	SIZE = atoi( argv[2] );
	int GPU = atoi( argv[3] );

	HitTile_float A;
	HitTile_float B;
	HitTile_float C;
	Timer tTotal;


	// 3. Declare and initialize full matrices and domains
	hit_tileDomainAlloc( &A, float, 2, SIZE, SIZE ); 
	hit_tileDomainAlloc( &B, float, 2, SIZE, SIZE );
	hit_tileDomainAlloc( &C, float, 2, SIZE, SIZE );


	/* INICIALIZA LAS MATRICES */
	for (int i=0; i < SIZE; i++){
		for (int j=0; j < SIZE; j++) {
			hit_tileElemAt(A, 2, i, j) = 18.345;
			hit_tileElemAt(B, 2, i, j) = 1.14;
			hit_tileElemAt(C, 2, i, j) = 0;
		}
	}

	cudaDeviceSynchronize();
	TimerStart(tTotal);

	// 4. Init computation threads
	CALThread threads;
	CALThreadInit( threads, 2, SIZE, SIZE );

	// 5. Create controller object
	CALCntrl comm;
	CAL_CntrlCreate(&comm, CAL_CNTRL_GPU, GPU);

	// 6. Attach data structures to the controllers
	CAL_CntrlAttach(&comm, (HitTile*)&A);
	CAL_CntrlAttach(&comm, (HitTile*)&B);
	CAL_CntrlAttach(&comm, (HitTile*)&C);

	// 7. Launch the task: invoking the kernel
	CAL_CntrlLaunch(comm, MatMult, threads, A, B, C);

	// 8. Copy result from device memory to host memory
	CAL_CntrlDetach(&comm, (HitTile*)&A);
	CAL_CntrlDetach(&comm, (HitTile*)&B);
	CAL_CntrlDetach(&comm, (HitTile*)&C);

	// 9. Destroy the controller
	CAL_CntrlDestroy(&comm);

	// 10. TIMES
	cudaDeviceSynchronize();
	TimerStop(tTotal);

	printf("SIZE %d \n", SIZE );
        printf("Clock Main %.8lf\n", TimerGetTime(tTotal));


        // 11. Calculate NORM
        double resultado=0,suma=0;
        for (int i=0; i<SIZE; i++){
                for (int j=0; j<SIZE; j++) {
                        #ifdef DEBUG
                                printf("%lf \n", hit_tileElemAt( C, 2, i, j ));
                        #endif
                         suma += pow(  hit_tileElemAt( C, 2, i, j ) ,2);
        }}
        printf("\n ----------------------- NORM ----------------------- \n");
        printf("\n Acumulated sum: %lf",suma);

        resultado=sqrt( suma );
        printf("\n Result: %lf \n",resultado);

        printf("\n ---------------------------------------------------- \n");

	// 12. Free data structures
	hit_tileFree(A);
	hit_tileFree(B);
	hit_tileFree(C);
	TimerDestroy(tTotal);
	
	// 13. Finish the controller library
        CAL_CntrlFinish();

	return 0;
}

