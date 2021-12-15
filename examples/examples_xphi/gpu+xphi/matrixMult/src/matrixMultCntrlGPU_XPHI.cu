//#define _BSD_SOURCE
#include <omp.h>
#include <hitmap.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "kernel_xphi.h"
#include "CAL_KernelGPU.h"
#include "CAL_KernelXPHI.h"


double  mainClock;
double  initClock;
double  sequentialClock;

#define BLOCKSIZE 32

#define SIZE 16

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



double t_start, t_end;

int main(int argc, char *argv[])
{

    #ifdef DEBUG
     setbuf(stdout, NULL);
     setbuf(stderr, NULL);
    #endif

//    CAL_CntrlInit(3);

    omp_set_nested(1);
    #pragma omp parallel num_threads(3)
    {
    #pragma omp single nowait
    {

    if ( argc < 4 ) {
                fprintf(stderr, "\nUsage: %s <numRows> <perGPU> <perXPHI> \n", argv[0]);
                exit(EXIT_FAILURE);
    }
    int rows = atoi( argv[1] );
    int columns = rows;
    float perGPU = atof( argv[2] );
    float perXPHI = atof( argv[3] );

    int i, j, k;

#ifdef DEBUG
printf("\n Rows: %d Columns: %d \n", rows,columns);
#endif

    
    /* 1. DIVIDE COMPUTATION */
    HitShape Xphi_shape=hit_shapeStd2(rows, columns); //HIT_SHAPE_NULL;
    HitShape GPU_shape=hit_shapeStd2(rows, columns); //HIT_SHAPE_NULL;
    HitLayout weightLay;


    /* 2. DECLARE FULL MATRIX AND COPY */
    HitTile_float matrixA, matrixB, matrixC;
    hit_tileDomainShapeAlloc( &matrixA, float,  Xphi_shape );
    hit_tileDomainShapeAlloc( &matrixB, float,  Xphi_shape );
    hit_tileDomainShapeAlloc( &matrixC, float,  Xphi_shape );
    //hit_tileDomain( &matrixC, float, 2, rows, columns );
    //hit_tileAlloc( &matrixC );

    HitTile_float matrixA_gpu, matrixB_gpu, matrixC_gpu;
    hit_tileDomainShapeAlloc( &matrixA_gpu, float, GPU_shape );
    hit_tileDomainShapeAlloc( &matrixB_gpu, float, GPU_shape );
    hit_tileDomainShapeAlloc( &matrixC_gpu, float, GPU_shape );
    

    CALCntrl comm;
    CALCntrl comm_gpu;
    CALThread threads;

    CALThreadInit(threads, 2, rows, columns);

    /* INIT MATRICES */
        int dim_0= hit_shapeSigCard(Xphi_shape,0);
        int dim_1= hit_shapeSigCard(Xphi_shape,1);

        for (int i=0; i<dim_0; i++){
                for (int j=0; j<dim_1; j++) {
                     hit_tileElemAt(matrixA, 2, i, j) = 18.345;
                     hit_tileElemAt(matrixB, 2, i, j) = 1.14;
                     hit_tileElemAt(matrixC, 2, i, j) = 0;
                }
        }

        int dim_0_GPU= hit_shapeSigCard(GPU_shape,0);
        int dim_1_GPU= hit_shapeSigCard(GPU_shape,1);
        for (int i=0; i<dim_0_GPU; i++){
                for (int j=0; j<dim_1_GPU; j++) {
                     hit_tileElemAt(matrixA_gpu, 2, i, j) = 18.345;
                     hit_tileElemAt(matrixB_gpu, 2, i, j) = 1.14;
                     hit_tileElemAt(matrixC_gpu, 2, i, j) = 0;
                }
        }

    //initMatrix( matrixA, matrixB, matrixC, rows, columns );
    //initMatrix( matrixA_gpu, matrixB_gpu, matrixC_gpu, rows, columns );


    /* CREATE CNTRLS */
    mainClock = omp_get_wtime();
   
    CAL_CntrlCreate(&comm_gpu, CAL_CNTRL_GPU, 0);
    CAL_CntrlCreate(&comm, CAL_CNTRL_XPHI, 228);
    
    CAL_CntrlAttach(&comm, (HitTile*)&matrixA);
    CAL_CntrlAttach(&comm, (HitTile*)&matrixB);
    CAL_CntrlAttach(&comm, (HitTile*)&matrixC);
    
    CAL_CntrlAttach(&comm_gpu, (HitTile*)&matrixA_gpu);
    CAL_CntrlAttach(&comm_gpu, (HitTile*)&matrixB_gpu);
    CAL_CntrlAttach(&comm_gpu, (HitTile*)&matrixC_gpu);

    CAL_CntrlGPUForceCntrlToHost( &comm_gpu, &matrixA_gpu );
    CAL_CntrlGPUForceCntrlToHost( &comm_gpu, &matrixB_gpu );
    CAL_CntrlGPUForceCntrlToHost( &comm_gpu, &matrixC_gpu );

   
    CAL_CntrlLaunch(comm, MatMul, threads, 3, &matrixA, &matrixB, &matrixC);	
    CAL_CntrlLaunch(comm_gpu, MatMult, threads, 3, &matrixA_gpu, &matrixB_gpu, &matrixC_gpu);


    CAL_CntrlDetach(&comm, (HitTile*)&matrixC);
    CAL_CntrlDetach(&comm_gpu, (HitTile*)&matrixC_gpu);

    CAL_CntrlDestroy(&comm_gpu);
    CAL_CntrlDestroy(&comm);


    cudaDeviceSynchronize(); 
    /* 4. CLOCK RESULTS */
    mainClock = omp_get_wtime() - mainClock;

    // TIMES
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock main: %lf\n", mainClock );



    // Calculate NORM
    double resultado=0,suma=0;
    for ( i=0; i<dim_0; i++ ) {
         for ( j=0; j<dim_1; j++ ) {
              suma += pow(  hit_tileElemAt( matrixC, 2, i, j ) ,2);
            }
    }
    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum XeonPHI: %lf",suma);
    resultado=sqrt( suma );
    printf("\n Result XeonXPHI: %lf \n",resultado);
    printf("\n ---------------------------------------------------- \n");
    
    //resultado=0; suma=0;
    for ( i=0; i<dim_0_GPU; i++ ) {
         for ( j=0; j<dim_1_GPU; j++ ) {
              suma += pow(  hit_tileElemAt( matrixC_gpu, 2, i, j ) ,2);
            }
    }
    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum GPU: %lf",suma);
    resultado=sqrt( suma );
    printf("\n Result GPU: %lf \n",resultado);
    printf("\n ---------------------------------------------------- \n");


    hit_tileFree(matrixA);
    hit_tileFree(matrixB);
    hit_tileFree(matrixC);
    hit_tileFree(matrixA_gpu);
    hit_tileFree(matrixB_gpu);
    hit_tileFree(matrixC_gpu);

    CAL_CntrlFinish();  

  return 0;
}
