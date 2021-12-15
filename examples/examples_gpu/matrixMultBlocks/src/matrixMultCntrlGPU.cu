#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "timer.h"
#include "CAL.h"

#define SEED 6834723

#ifndef BLOCKSIZE
#define BLOCKSIZE 32
#endif


hit_tileNewType( float );
hit_ktileNewType( float );

/* A. GPU Kernel charazterization */
CAL_KERNEL_GPU_CHAR_STATIC(MatMult, 2, fixed, square, 32);

/* B.1 Generic Kernel implementation */
CAL_KERNEL(MatMult, dGPU, KHitTile_float A, KHitTile_float B, KHitTile_float C){

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
	//for (int m = 0; m < (hit_tileDimCard(*A, 1) / BLOCKSIZE); ++m) 
	for (int m = 0; m < A.origAcumCard[0] / BLOCKSIZE; ++m) {

		KHitTile_float subA = hit_ktileSelect2( A, blockIdx.y * BLOCKSIZE, BLOCKSIZE, m * BLOCKSIZE, BLOCKSIZE );
		KHitTile_float subB = hit_ktileSelect2( B, m * BLOCKSIZE, BLOCKSIZE, blockIdx.x * BLOCKSIZE, BLOCKSIZE );

		// Shared memory used to store Asub and Bsub respectively
		__shared__ float As[BLOCKSIZE][BLOCKSIZE];
		__shared__ float Bs[BLOCKSIZE][BLOCKSIZE];

		// Load Asub and Bsub from device memory to shared memory
		// Each thread loads one element of each sub-matrix
		As[row][col] = hit_tileElemAtNoStride(subA, 2, threadIdx.y, threadIdx.x);
		Bs[row][col] = hit_tileElemAtNoStride(subB, 2, threadIdx.y, threadIdx.x);

		// Synchronize to make sure the sub-matrices are loaded
		// before starting the computation
		__syncthreads();
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
	hit_tileElemAtNoStride(C, 2, threadId.y, threadId.x) = Cvalue;
}

/* C. Defining kernel prototypes */
CAL_KERNEL_PROTO( MatMult,                                               // Name
                  1, dGPU,                                                // Implementations
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
	if ( argc!=3 ) {
		fprintf(stderr, "Usage: %s <matrixSize> <GPU>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int SIZE = atoi( argv[1] );
	int GPU = atoi( argv[2] );

	HitTile_float A;
	HitTile_float B;
	HitTile_float C;
	Timer tTotal;

        // 3. Declare and initialize full matrices and domains
	hit_tileDomainAlloc( &A, float, 2, SIZE, SIZE ); 
	hit_tileDomainAlloc( &B, float, 2, SIZE, SIZE );
	hit_tileDomainAlloc( &C, float, 2, SIZE, SIZE );


	srand48( SEED );
	TimerCreate(tTotal);

	/* INICIALIZA LAS MATRICES */
#ifdef NDEBUG
	for (int i=0; i < SIZE; i++){
		for (int j=0; j < SIZE; j++) {
			hit_tileElemAt(A, 2, i, j) = (float)drand48();
			hit_tileElemAt(B, 2, i, j) = (float)drand48();
			hit_tileElemAt(C, 2, i, j) = 0;
		}
	}
#else
	for (int i=0; i < SIZE; i++){
		for (int j=0; j < SIZE; j++) {
			hit_tileElemAt(A, 2, i, j) = 18.345;
			hit_tileElemAt(B, 2, i, j) = 1.14;
			hit_tileElemAt(C, 2, i, j) = 0;
		}
	}
#endif

	cudaDeviceSynchronize();
	TimerStart(tTotal);

        // 4. Init computation threads
	CALThread threads;
	CALThreadInit( threads, 2, SIZE, SIZE );
        
	// 5. Create controller object
	CALCntrl cntrl;
	CAL_CntrlCreate(&cntrl, CAL_CNTRL_GPU, GPU);

        // 6. Attach data structures to the controllers
	CAL_CntrlAttach(&cntrl, (HitTile*)&A);
	CAL_CntrlAttach(&cntrl, (HitTile*)&B);
	CAL_CntrlAttach(&cntrl, (HitTile*)&C);

	// 7. Launch the task: invoking the kernel	
	CAL_CntrlLaunch(cntrl, MatMult, threads, A, B, C);

	// 8. Copy result from device memory to host memory
	CAL_CntrlDetach(&cntrl, (HitTile*)&A);
	CAL_CntrlDetach(&cntrl, (HitTile*)&B);
	CAL_CntrlDetach(&cntrl, (HitTile*)&C);

        // 9. Destroy the controller
        CAL_CntrlDestroy(&cntrl);

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

