#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "CAL.h"

#define SEED 6834723


hit_tileNewType( float );
hit_ktileNewType( float );

/* A. GPU Kernel charazterizations */
CAL_KERNEL_GPU_CHAR_STATIC(Copy, 2, full, low, low);
CAL_KERNEL_GPU_CHAR_STATIC(Update, 2, medium, medium, medium);

/* B.1 GPU Kernel implementation */
CAL_KERNEL(Copy, dGPU, KHitTile_float dst, KHitTile_float src){
	int row = threadId.y;
	int col = threadId.x;
	hit_tileElemAtNoStride(dst, 2, row, col) = hit_tileElemAtNoStride(src, 2, row, col);
}

/* B.2 GPU Kernel implementation */
CAL_KERNEL(Update, dGPU, KHitTile_float dst, KHitTile_float src){
	int row = threadId.y + 1;
	int col = threadId.x + 1;
	hit_tileElemAtNoStride(dst, 2, row, col) = ( 
			hit_tileElemAtNoStride(src, 2, row - 1, col    ) +
			hit_tileElemAtNoStride(src, 2, row + 1, col    ) +
			hit_tileElemAtNoStride(src, 2, row    , col - 1) +
			hit_tileElemAtNoStride(src, 2, row    , col + 1) ) / 4;
}

/* C. Defining kernel prototypes */
CAL_KERNEL_PROTO( Copy,                                               // Name
                  1, dGPU,                                   // Implementations
                  2, OUT, HitTile_float, dst, IN, HitTile_float, src
                );

CAL_KERNEL_PROTO( Update,                                               // Name
                  1, dGPU,                                   // Implementations
                  2, OUT, HitTile_float, dst, IN, HitTile_float, src
                );

/*
 * Main program to perform cellular automata 
 */
int main(int argc, char* argv[]) {

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
	if ( argc != 4 ) {
		fprintf(stderr, "Usage: %s <matrixSize> <numStages> <GPU>\n", argv[0] );
		exit( EXIT_FAILURE );
	}
	int SIZE = atoi(argv[1]);
	int STAGES = atoi(argv[2]);
	int GPU = atoi(argv[3]);

	HitTile_float mat;
	HitTile_float copy;
	Timer tTotal;
	Timer tKernel;

        // 3. Declare and initialize full matrices and domains
	hit_tileDomain ( &mat, float, 2, SIZE, SIZE );
	hit_tileDomain ( &copy, float, 2, SIZE, SIZE );
	hit_tileAlloc( &mat );
	//hit_tileAlloc( &copy );

	srand( SEED );
	TimerCreate(tTotal);
	TimerCreate(tKernel);

	for ( int i=0; i<SIZE; i++ )
		for ( int j=0; j<SIZE; j++ )
			hit_tileElemAt( mat, 2, i, j ) = 0.0;

	for ( int j=0; j<SIZE; j++ ) {
		hit_tileElemAt( mat, 2, 0, j ) = 1;
		hit_tileElemAt( mat, 2, SIZE-1, j ) = 2;
	}
	for ( int i=0; i<SIZE; i++ ) {
		hit_tileElemAt( mat, 2, i, 0 ) = 3;
		hit_tileElemAt( mat, 2, i, SIZE-1 ) = 4;
	}

	cudaDeviceSynchronize();
	TimerStart(tTotal);

        // 4. Init computation threads
	CALThread threads;
	CALThreadInit(threads, 2, SIZE, SIZE);
	CALThread update;
	CALThreadInit(update, 2, SIZE-2, SIZE-2)

        // 5. Create controller object
	CALCntrl comm;
	CAL_CntrlCreate(&comm, CAL_CNTRL_GPU, GPU);

        // 6. Attach data structures to the controllers
	CAL_CntrlAttach(&comm, (HitTile*)&mat);
//	CAL_CntrlAttach(&comm, (HitTile*)&copy);
	CAL_CntrlInternal(&comm, (HitTile*)&copy);

        // 7. Launch the task: invoking the kerneli
	for (int i = 0; i < STAGES; i++){
		// Update copy
		CAL_CntrlLaunch(comm, Copy, threads, copy, mat);
		
		// Compute iteration
		CAL_CntrlLaunch(comm, Update, update, mat, copy);
	}
        
	// 8. Copy result from device memory to host memory
	CAL_CntrlDetach(&comm, (HitTile*)&mat);
	
	// 9. Destroy the controller
	CAL_CntrlDestroy(&comm);

	// 10. Times
	cudaDeviceSynchronize();
	TimerStop(tTotal);
        printf("SIZE %d %d\n", SIZE, STAGES );
        printf("Clock Main %.8lf\n", TimerGetTime(tTotal));

        // 11. Calculate NORM
        double resultado=0,suma=0;
        for (int i=0; i<SIZE; i++){
                for (int j=0; j<SIZE; j++) {
                       // #ifdef DEBUG
                                printf("%lf \n", hit_tileElemAt( mat, 2, i, j ));
                        //#endif
                         suma += pow(  hit_tileElemAt( mat, 2, i, j ) ,2);
        }}
        printf("\n ----------------------- NORM ----------------------- \n");
        printf("\n Acumulated sum: %lf",suma);

        resultado=sqrt( suma );
        printf("\n Result: %lf \n",resultado);

        printf("\n ---------------------------------------------------- \n");

	
	//printf("Tiempo kernels: %.8lf\n",TimerGetTime(tKernel));
	
	// 12. Free data structures
	hit_tileFree(mat);
	TimerDestroy(tTotal);
	TimerDestroy(tKernel);
 
        // 13. Finish the controller library
        CAL_CntrlFinish();

	return 0;
}
