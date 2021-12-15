#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "timer.h"
#include "CAL.h"
#include "CAL_KernelGPU.h"
#include "CAL_KernelCPU.h"
#include "CAL_CntrlDev.h"



//hit_tileNewType( float );

#ifndef BLOCKSIZE
#define BLOCKSIZE 32
#endif


/* GPU KERNELS */

CAL_KERNEL_GPU_CHAR_STATIC(MatMult, 2, fixed, square, 32);
CAL_KERNEL_GPU(MatMult, 3, IN, HitTile_float*, A, IN, HitTile_float*, B, IO, HitTile_float*, C){

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
                iter < A.dimy / BLOCKSIZE;
                iter++, aCol+=BLOCKSIZE, bRow+=BLOCKSIZE )
        {
                // Shared memory used to store Asub and Bsub respectively
                __shared__ float As[BLOCKSIZE][BLOCKSIZE];
                __shared__ float Bs[BLOCKSIZE][BLOCKSIZE];

                // Load the matrices from device memory
                // to shared memory; each thread loads
                // one element of each matrix
                As[row][col] = hit_ktileElemAt( A, 2, threadId.y, aCol + col );
                Bs[row][col] = hit_ktileElemAt( B, 2, bRow + row, bCol + col );

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
        hit_ktileElemAt(C, 2, threadId.y, threadId.x) = Cvalue;

}


HitClock seqClock;
HitClock mainClock;
HitClock commClock;


/**
 MAIN PROGRAM 
*/
int main(int argc, char *argv[]) {

	setbuf(stdout, NULL);

	omp_set_nested(1);
  	#pragma omp parallel num_threads(2)
	{
	#pragma omp single nowait
	{

	if ( argc<6 ) {
		fprintf(stderr, "Usage: %s <matrixSize> <numIter> <num_cpus> <per CPU> <per GPU>i, numIter unuseful\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int SIZE = atoi( argv[1] );
	int num_cpus = atoi( argv[3] );
	int perCPU = atoi( argv[4] );
	int perGPU = atoi( argv[5] );

	// Needed internally by the controllers
	hit_clockReset(seqClock);
	hit_clockReset(commClock);
        hit_clockReset(mainClock);



	// 1. Part the domain in two 
	HitShape domain= hit_shapeStd2(SIZE, SIZE);
	HitShape domains_g[2]={domain, domain};
    	HitShape domains_cntrl[2];
    	hit_Cntrlpart(domains_cntrl, domain, perCPU,perGPU);

#ifdef DEBUG    	
	dumpShape(domains_cntrl[0]);
    	dumpShape(domains_cntrl[1]);
	printf("\n Cards: %d %d\n", hit_shapeSigCard(domains_cntrl[1],0), hit_shapeSigCard(domains_cntrl[1],1)); 
#endif



	// 2. Determine the threads to launch. TODO with a layout
	CALThreadDev threads; 
	CALThreadInitDev(2,threads, domains_cntrl);
	CALThreadReverse2(threads, 1 );
	CALThreadReverse2(threads, 0 );

	// 3. Create the multiple-device controller
	CALMCntrl cntrlMult;
	CAL_MCntrlCreate2(cntrlMult, CAL_CNTRL_GPU, 0, CAL_CNTRL_GPU, 1 );	

	// 4. Define data structures shared between several devices in a node
	CHitTile_float A, B, C; 

	// Data structures, number of devices, type, how part the structure
	/*hit_CtileDomainAlloc(A, 2, float, domains_cntrl);	
	hit_CtileDomainAlloc(B, 2, float, domains_cntrl);	
	hit_CtileDomainAlloc(C, 2, float, domains_cntrl);	
	*/
	// TODO All matrix are allocated in both CPU and GPU
	hit_CtileDomainAlloc(A, 2, float, domains_g);	
	hit_CtileDomainAlloc(B, 2, float, domains_g);	
	hit_CtileDomainAlloc(C, 2, float, domains_g);	

	// Initialize
        for (int i=0; i < SIZE; i++){
                for (int j=0; j < SIZE; j++) {
                        hit_tileElemAt(A.DS_dev[0], 2, i, j) = 0.345;
                        hit_tileElemAt(A.DS_dev[1], 2, i, j) = 0.345;
                        hit_tileElemAt(B.DS_dev[0], 2, i, j) = 1.14;
                        hit_tileElemAt(B.DS_dev[1], 2, i, j) = 1.14;
                        hit_tileElemAt(C.DS_dev[0], 2, i, j) = 0;
                        hit_tileElemAt(C.DS_dev[1], 2, i, j) = 0;
                }
        }	

#ifdef DEBUG
        int deviceCount = 0;
        cudaGetDeviceCount(&deviceCount);
        printf("\n GPUS: %d\n", deviceCount);
#endif

        cudaSetDevice(0);
        cudaDeviceSynchronize();
        cudaSetDevice(1);
        cudaDeviceSynchronize();
        hit_clockStart( mainClock );
        


	// 5. Attach the data structures to a multiple-device controller
	CAL_MCntrlAttach(A, cntrlMult);
	CAL_MCntrlAttach(B, cntrlMult);
	CAL_MCntrlAttach(C, cntrlMult);
	
	//CUDA_CHECK();

	// 6. Invoke kernels

	cudaSetDevice(0);
        cudaDeviceSynchronize();
        cudaSetDevice(1);
        cudaDeviceSynchronize();
        hit_clockStart( seqClock );


	// Compute
	CAL_MCntrlLaunch(cntrlMult, threads,0, MatMult, 3, &hit_Ctile(A,0), &hit_Ctile(B,0), &hit_Ctile(C,0));
	CAL_MCntrlLaunch(cntrlMult, threads,1, MatMult, 3, &hit_Ctile(A,1), &hit_Ctile(B,1), &hit_Ctile(C,1));

	cudaSetDevice(0);
        cudaDeviceSynchronize();
        cudaSetDevice(1);
        cudaDeviceSynchronize();
        hit_clockStop( seqClock );

	// 7. Copy result from device memory to host memory
	//CAL_MCntrlDetach(A, cntrlMult);
	//CAL_MCntrlDetach(B, cntrlMult);
	CAL_MCntrlDetach(C, cntrlMult);

	
	// 8. Destroy multiple-device controller	
	CAL_MDestroy2(cntrlMult);

        cudaSetDevice(0);
        cudaDeviceSynchronize();
        cudaSetDevice(1);
        cudaDeviceSynchronize();
        hit_clockStop( mainClock );

        printf("\nmatrixMult SIZE: %d\n", SIZE );
        printf("\n%d-GPU-0: %d%\tGPU-1: %d%\n", num_cpus, perCPU, perGPU );
        printf("Clock main: %.8lf\n", mainClock.seconds);
        printf("Clock seq: %.8lf\n", seqClock.seconds);
        printf("Clock comm: %.8lf\n", commClock.seconds);

        
	// Calculate NORM
	int dim_0= hit_shapeSigCard(domains_cntrl[1],0);
	int dim_1= hit_shapeSigCard(domains_cntrl[1],1);

	int dim_0_CPU= hit_shapeSigCard(domains_cntrl[0],0);
	int dim_1_CPU= hit_shapeSigCard(domains_cntrl[0],1);

        double resultado=0,suma=0;
	int i, j;
        for ( i=0; i<dim_0_CPU; i++ ) {
                for ( j=0; j<dim_1_CPU; j++ ) {
                  suma += pow(  hit_tileElemAt( C.DS_dev[0], 2, i, j ) ,2);
                }
        }
        for ( i=0; i<dim_0; i++ ) {
                for ( j=0; j<dim_1; j++ ) {
                  suma += pow(  hit_tileElemAt( C.DS_dev[1], 2, i, j ) ,2);
                }
        }
        printf("\n ----------------------- NORM ----------------------- \n");
        printf("\n Acumulated sum: %lf",suma);

        resultado=sqrt( suma );
        printf("\n Result: %lf \n",resultado);

        printf("\n ---------------------------------------------------- \n");

#ifdef NDEBUG

	printf("\n Checking\n");
        for (int i=0; i<dim_0; i++)
                for (int j=0; j<dim_1; j++) 
                        if ( fabs( hit_tileElemAt(C.DS_dev[1], 2, i, j) - 18.345 * 1.14 * SIZE ) > 0.01 )        
				fprintf(stderr, "ERROR: Resultado C[%d,%d] = %lf \n", i, j, 
							hit_tileElemAt((C.DS_dev[1]), 2, i, j) );
		
        for (int i=0; i<dim_0_CPU; i++)
                for (int j=0; j<dim_1_CPU; j++) 
			if ( fabs( hit_tileElemAt(C.DS_dev[0], 2, i, j) - 18.345 * 1.14 * SIZE ) > 0.01 )
                                fprintf(stderr, "ERROR: Resultado C_CPU[%d,%d] = %f \n", i, j, 
							hit_tileElemAt((C.DS_dev[0]), 2, i, j) );
	
	

#endif
	
	// 9. Free resources
	hit_CtileFree(A, cntrlMult);
	hit_CtileFree(B, cntrlMult);
	hit_CtileFree(C, cntrlMult);
	

}}

	return 0;
}
