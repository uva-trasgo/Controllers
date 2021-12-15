#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "timer.h"
#include "CAL.h"

#define SEED 6834723

Timer tInner;

#define CUDA_CHECK() { cudaError_t error; if ((error = cudaGetLastError())!=cudaSuccess){ fprintf(stdout,"%s: %s\n", cudaGetErrorName(error),cudaGetErrorString(error));exit(EXIT_FAILURE); } }

hit_tileNewType( float );
hit_ktileNewType( float );


/* A. GPU Kernel charazterization */
CAL_KERNEL_GPU_CHAR_STATIC(MatSum, 2, full, low, low);

/* B.1 Generic Kernel implementation */
CAL_KERNEL(MatSum, Generic, int numIter, KHitTile_float A, KHitTile_float B, KHitTile_float C){
	int i = threadId.y;
	int j = threadId.x;
	for (int iter=0; iter<numIter; iter++) 
		hit_tileElemAtNoStride(C, 2, i, j) = 
			hit_tileElemAtNoStride(C, 2, i, j) +
			hit_tileElemAtNoStride(A, 2, i, j) + 
			hit_tileElemAtNoStride(B, 2, i, j);
	//printf("Generic: %lf\n", hit_tileElemAtNoStride(C, 2, i, j));
}

/* B.2 GPU Kernel implementation */
CAL_KERNEL(MatSum, dGPU, int numIter, KHitTile_float A, KHitTile_float B, KHitTile_float C){
	int i = threadId.y;
	int j = threadId.x;
	for (int iter=0; iter<numIter; iter++) 
		hit_tileElemAtNoStride(C, 2, i, j) = 
			hit_tileElemAtNoStride(C, 2, i, j) +
			hit_tileElemAtNoStride(A, 2, i, j) + 
			hit_tileElemAtNoStride(B, 2, i, j);
	//printf("dGPU: %lf+ %lf = %lf\n", hit_tileElemAtNoStride(A, 2, i, j), hit_tileElemAtNoStride(B, 2, i, j), hit_tileElemAtNoStride(C, 2, i, j));
}


/* C. Defining kernel prototypes */ 
CAL_KERNEL_PROTO( MatSum,                                               // Name
                  2, dGPU,  Generic,                                  // Implementations
		  4, INVAL, int, numIter, IN, HitTile_float, A, IN, HitTile_float, B, OUT, HitTile_float, C 
                );

/*
 * Main program to perform matrix addition
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
		fprintf(stderr, "Usage: %s <matrixSize> <numIter> <GPU>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int SIZE = atoi( argv[1] );
	int NITER = atoi( argv[2] );
	int GPU = atoi( argv[3] );

	HitTile_float A;
	HitTile_float B;
	HitTile_float C;
	Timer tTotal;

	// 3. Declare and initialize full matrices and domains
	hit_tileDomain( &A, float, 2, SIZE, SIZE ); 
	hit_tileDomain( &B, float, 2, SIZE, SIZE );
	hit_tileDomain( &C, float, 2, SIZE, SIZE );

	hit_tileAlloc( &A );
	hit_tileAlloc( &B );
	hit_tileAlloc( &C );

	for (int i=0; i < SIZE; i++){
		for (int j=0; j < SIZE; j++) {
			hit_tileElemAt(A, 2, i, j) = 2.12;
			hit_tileElemAt(B, 2, i, j) = 0.2;
			hit_tileElemAt(C, 2, i, j) = 0;
		}
	}

	cudaSetDevice(GPU);
	cudaDeviceSynchronize();
	TimerCreate(tTotal);
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

	// 7. Launch the task: invoking the kerneli
	CAL_CntrlLaunch(comm, MatSum, threads, NITER, A, B, C);

	// 8. Copy result from device memory to host memory
	CAL_CntrlDetach(&comm, (HitTile*)&A);
	CAL_CntrlDetach(&comm, (HitTile*)&B);
	CAL_CntrlDetach(&comm, (HitTile*)&C);

	// 9. Destroy the controller
    	CAL_CntrlDestroy(&comm);

	// 10. TIMES
	cudaDeviceSynchronize();
	TimerStop(tTotal);

	printf("SIZE %d %d\n", SIZE, NITER );
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
