// System includes
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "CAL.h"

#ifndef BLOCKSIZE
#define BLOCKSIZE 32
#endif

hit_tileNewType( float );
hit_ktileNewType( float );
int GPU=0;

/* A. GPU Kernel charazterizations */
CAL_KERNEL_GPU_CHAR_STATIC(Copy, 2, full, low, low);
CAL_KERNEL_GPU_CHAR_STATIC(MatMult, 2, fixed, square, 32);

/* B. GPU Kernel implementation */
CAL_KERNEL(Copy, dGPU, KHitTile_float dst, KHitTile_float src){
	int row = threadId.y;
	int col = threadId.x;
	if (row < dst.origAcumCard[1] && col < dst.origAcumCard[1])
		hit_tileElemAtNoStride(dst, 2, row, col) = hit_tileElemAtNoStride(src, 2, row, col);
}

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
	hit_tileElemAtNoStride(C, 2, threadId.y, threadId.x) = hit_tileElemAtNoStride(C, 2, threadId.y, threadId.x)+ Cvalue;
}


CAL_KERNEL_PROTO( MatMult,                                               // Name
                  1, dGPU,	  	                                  // Implementations
                  3, IN, HitTile_float, A, IN, HitTile_float, B, OUT, HitTile_float, C
                );


/* C. Defining kernel prototypes */
CAL_KERNEL_PROTO( Copy,                                               	// Name
                  1, dGPU,                                   // Implementations
                  2, OUT, HitTile_float, dst, IN, HitTile_float, src
                );

/**
 * Program main
 */
int main(int argc, char **argv)
{

        // 1. Init Controllers library									
        omp_set_nested(1);
        omp_set_num_threads(3);
        #pragma omp parallel 
        {
        #pragma omp single 
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
    
    /*Timer*/
    Timer tTotal, tTotalA;
	Timer tComp;
	Timer tComm;
	TimerCreate(tTotal); TimerCreate(tTotalA);
	TimerCreate(tComp);
	TimerCreate(tComm);

  
    // Use a larger block size for Fermi and above
    int block_size =  32;

	/*Variables*/
	HitTile_float A;
	HitTile_float B;
	HitTile_float B2;
	HitTile_float C;

	hit_tileDomain ( &A, float, 2, SIZE, SIZE );
	hit_tileDomain ( &B, float, 2, SIZE, SIZE );
	hit_tileDomain ( &B2, float, 2, SIZE, SIZE );
	hit_tileDomain ( &C, float, 2, SIZE, SIZE );
	hit_tileAlloc( &A ); hit_tileAlloc( &B ); hit_tileAlloc( &B2 ); hit_tileAlloc( &C );

	for (int i = 0; i < SIZE*SIZE; i++)
		{	hit_tileElemAt( A, 1, i) = 1.0; 
			hit_tileElemAt( B, 1, i) = 2.0; 
			hit_tileElemAt( B2, 1, i) = 3.0; 
			hit_tileElemAt( C, 1, i)=0.0;}
		
	CALThread threads;
	CALThreadInit(threads, 2, SIZE, SIZE);		


	// Controller
	CALCntrl comm;
	CAL_CntrlCreate(&comm, CAL_CNTRL_GPU, GPU);

TimerStart( tTotal ); TimerStart( tComm );	
	/*Comunicaición*/
	CAL_CntrlAttach(&comm, (HitTile*)&A);
	CAL_CntrlAttach(&comm, (HitTile*)&B);
	CAL_CntrlAttach(&comm, (HitTile*)&B2);
	CAL_CntrlAttach(&comm, (HitTile*)&C);
	CAL_CntrlSync(comm);
TimerStop( tComm );

TimerStart( tComp );
	CAL_CntrlLaunch(comm, MatMult, threads, A, B, C);
	CAL_CntrlLaunch(comm, Copy, threads, A, C);
	CAL_CntrlLaunch(comm, MatMult, threads, A, B2, C);
	CAL_CntrlSync(comm);
TimerStop( tComp );

TimerContinue( tComm );
	CAL_CntrlDetach(&comm, (HitTile*)&C);
	CAL_CntrlSync(comm);
TimerStop( tTotal ); TimerStop( tComm ); 	
	CAL_CntrlDestroy(&comm);

		printf("\nVERSION SINCRONA************\n" );
        printf("SIZE %d\n", SIZE);
        printf("Clock Main %.8lf <--\n", TimerGetTime(tTotal));
        printf("Clock Comm %.8lf\n", TimerGetTime(tComm));
        printf("Clock Comp %.8lf\n", TimerGetTime(tComp));
		printf("Resultado %lf\n", hit_tileElemAt( C, 1, 0));
		
	hit_tileFree(A); hit_tileFree(B); hit_tileFree(B2); hit_tileFree(C);

	/*Version asincrona*/												///////////////////////////////////

	/*Variables*/
	HitTile_float AA;
	HitTile_float BA;
	HitTile_float B2A;
	HitTile_float CA;

	hit_tileDomain ( &AA, float, 2, SIZE, SIZE );
	hit_tileDomain ( &BA, float, 2, SIZE, SIZE );
	hit_tileDomain ( &B2A, float, 2, SIZE, SIZE );
	hit_tileDomain ( &CA, float, 2, SIZE, SIZE );
	hit_tileAlloc( &AA ); hit_tileAlloc( &BA ); hit_tileAlloc( &B2A ); hit_tileAlloc( &CA );

	for (int i = 0; i < SIZE*SIZE; i++)
		{	hit_tileElemAt( AA, 1, i) = 1.0; 
			hit_tileElemAt( BA, 1, i) = 2.0; 
			hit_tileElemAt( B2A, 1, i) = 3.0; 
			hit_tileElemAt( CA, 1, i)=0.0;}

	// Controller
	CALCntrl commA;
	CAL_CntrlCreate(&commA, CAL_CNTRL_GPU, GPU);

	cudaStream_t stream1, stream2;
	cudaStreamCreate ( &stream1) ; cudaStreamCreate ( &stream2) ; 	
	
	CAL_CntrlAttachAsyn(&commA, (HitTile*)&AA, stream1);
	CAL_CntrlAttachAsyn(&commA, (HitTile*)&BA, stream1);
	CAL_CntrlAttachAsyn(&commA, (HitTile*)&B2A, stream1);
	CAL_CntrlAttachAsyn(&commA, (HitTile*)&CA, stream1);
	CAL_CntrlSync(commA);
TimerStart( tTotalA );
	CAL_CntrlMoveToAsyn(&commA, (HitTile*)&AA, stream1); 
	CAL_CntrlMoveToAsyn(&commA, (HitTile*)&BA, stream1); 
	CAL_CntrlMoveToAsyn(&commA, (HitTile*)&CA, stream1); 

	CAL_CntrlLaunchCKernel(commA, MatMult, threads, stream1, AA, BA, CA);
	CAL_CntrlLaunchCKernel(commA, Copy, threads, stream1, AA, CA);

	CAL_CntrlMoveToAsyn(&commA, (HitTile*)&B2A, stream2);
	CAL_CntrlSync(commA);
	CAL_CntrlLaunchCKernel(commA, MatMult, threads, stream2, AA, B2A, CA);
	
	CAL_CntrlDetachAsyn(&commA, (HitTile*)&CA, stream2);
	CAL_CntrlSync(commA);
TimerStop( tTotalA );	

	CAL_CntrlDestroy(&commA);
	
		printf("\nVERSION A_SINCRONA************\n" );
        printf("SIZE %d\n", SIZE);
        printf("Clock Main %.8lf <--\n", TimerGetTime(tTotalA));
		printf("Resultado %lf\n", hit_tileElemAt( CA, 1, 0));
        printf("\nPorcentaje de mejora (%d x %d;) %.3lf%\n", SIZE, SIZE, (100.0 - (TimerGetTime(tTotalA)*100.0)/TimerGetTime(tTotal)));
	
	
	
	cudaStreamDestroy ( stream1 );	cudaStreamDestroy ( stream2 );	
	hit_tileFree(AA); hit_tileFree(BA); hit_tileFree(B2A); hit_tileFree(CA);

	CAL_CntrlFinish();

    return (0);
}
