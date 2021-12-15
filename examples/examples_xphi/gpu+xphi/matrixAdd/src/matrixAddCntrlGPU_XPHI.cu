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


void dumpShape(HitShape sh){

        int i;
        printf("[%d:%d:%d",hit_shapeSig(sh,0).begin,hit_shapeSig(sh,0).end,hit_shapeSig(sh,0).stride);
        for(i=1;i<hit_shapeDims(sh);i++) printf(",%d:%d:%d",hit_shapeSig(sh,i).begin,hit_shapeSig(sh,i).end,hit_shapeSig(sh,i).stride);
        printf("] \t cards: [%d",hit_sigCard(hit_shapeSig(sh,0)));
        for(i=1;i<hit_shapeDims(sh);i++) printf(",%d",hit_sigCard(hit_shapeSig(sh,i)));
        printf("]\n");
}


double  mainClock;
double  initClock;
double  sequentialClock;


#define SIZE 16

CAL_KERNEL_GPU_CHAR_STATIC(MatAdd, 2, full, low, low);
CAL_KERNEL_GPU(MatAdd, 3, IN, HitTile_float*, A, IN, HitTile_float*, B, IO, HitTile_float*, C){
/* Version KTiles */
/* Same best performance using ktile notation. Register indexes i,j obtain the last littel
        performance improvement, beyond the reference code */
        int i = threadId.y;
        int j = threadId.x;
                hit_ktileElemAt(C, 2, i, j) =
                        hit_ktileElemAt(C, 2, i, j) +
                        hit_ktileElemAt(A, 2, i, j) +
                        hit_ktileElemAt(B, 2, i, j);

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
printf("\n GPU: %lf MIC: %lf \n", perGPU, perXPHI);
#endif

    
    /* 1. DIVIDE COMPUTATION */
    HitLayout weightLay;
    // 1 para decir que es solo 1 dimension y va a ser partida en 4 segmentos
    int dummyProcElems[1] = {2};
    HitTopology dummyTopology = hit_topology(plug_topDummyDims, 1, dummyProcElems);
    HitShape globalShape = hit_shapeStd2(rows, columns);
    // The dimension where the partition is performed
    int restrictDim=0; 
    float weights[2] = {perGPU,perXPHI};
    weightLay = hit_layout(plug_layDimWeighted, dummyTopology, globalShape, restrictDim, weights);
    
    HitShape Xphi_shape = hit_layShapeNeighbor(weightLay , 0, 1);
    HitShape GPU_shape = hit_layShape(weightLay); 
 
 
    //HitShape Xphi_shape=hit_shapeStd2(rows, columns); //HIT_SHAPE_NULL;
    //HitShape GPU_shape=hit_shapeStd2(rows, columns); //HIT_SHAPE_NULL;

    dumpShape(Xphi_shape); 
    dumpShape(GPU_shape); 

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
    int dim_0= hit_shapeSigCard(Xphi_shape,0);
    int dim_1= hit_shapeSigCard(Xphi_shape,1);
    int dim_0_GPU= hit_shapeSigCard(GPU_shape,0);
    int dim_1_GPU= hit_shapeSigCard(GPU_shape,1);
    

    CALCntrl comm;
    CALCntrl comm_gpu;
    CALThread threads;
    CALThread threads_gpu;

    CALThreadInit(threads, 2, dim_0, dim_1);
    CALThreadInit(threads_gpu, 2, dim_1_GPU, dim_0_GPU);

    /* INIT MATRICES */

        for (int i=0; i<dim_0; i++){
                for (int j=0; j<dim_1; j++) {
                     hit_tileElemAt(matrixA, 2, i, j) = 0.12;
                     hit_tileElemAt(matrixB, 2, i, j) = 0.2;
                     hit_tileElemAt(matrixC, 2, i, j) = 0;
                }
        }

        for (int i=0; i<dim_0_GPU; i++){
                for (int j=0; j<dim_1_GPU; j++) {
                     hit_tileElemAt(matrixA_gpu, 2, i, j) = 0.12;
                     hit_tileElemAt(matrixB_gpu, 2, i, j) = 0.2;
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

    if(dim_0> 0 && dim_1>0){
    	CAL_CntrlLaunch(comm, MatSum, threads, 3, &matrixA, &matrixB, &matrixC);	
    }
    if(dim_0_GPU> 0 && dim_1_GPU>0){
        CAL_CntrlLaunch(comm_gpu, MatAdd, threads_gpu, 3, &matrixA_gpu, &matrixB_gpu, &matrixC_gpu);
    }

    CAL_CntrlDetach(&comm, (HitTile*)&matrixC);
    CAL_CntrlDetach(&comm_gpu, (HitTile*)&matrixC_gpu);

    CAL_CntrlDestroy(&comm);
    CAL_CntrlDestroy(&comm_gpu);


    cudaDeviceSynchronize(); 
    /* 4. CLOCK RESULTS */
    mainClock = omp_get_wtime() - mainClock;

    // TIMES
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock main: %lf\n", mainClock );


#ifdef DEBUG
    for ( i=0; i<dim_0; i++ ) {
         for ( j=0; j<dim_1; j++ ) {
                  printf("%lf ", hit_tileElemAt( matrixC, 2, i, j ));
            }
        printf("\n");
   }
    for ( i=0; i<dim_0_GPU; i++ ) {
         for ( j=0; j<dim_1_GPU; j++ ) {
                  printf("%lf ", hit_tileElemAt( matrixC_gpu, 2, i, j ));
            }
        printf("\n");
   }
#endif



    // Calculate NORM
    double resultado=0,suma=0;
    double resultadoT=0,sumaT=0;
    for ( i=0; i<dim_0; i++ ) {
         for ( j=0; j<dim_1; j++ ) {
              suma += pow(  hit_tileElemAt( matrixC, 2, i, j ) ,2);
            }
    }
    resultadoT+=resultado;
    sumaT+=suma;

    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum XeonPHI: %lf",suma);
    resultado=sqrt( suma );
    printf("\n Result XeonXPHI: %lf \n",resultado);
    printf("\n ---------------------------------------------------- \n");
    
    resultado=0; suma=0;
    for ( i=0; i<dim_0_GPU; i++ ) {
         for ( j=0; j<dim_1_GPU; j++ ) {
              suma += pow(  hit_tileElemAt( matrixC_gpu, 2, i, j ) ,2);
            }
    }
    resultadoT+=resultado;
    sumaT+=suma;
    
    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum GPU: %lf",suma);
    resultado=sqrt( suma );
    printf("\n Result GPU: %lf \n",resultado);
    printf("\n ---------------------------------------------------- \n");


    printf("\n ----------------------- NORM TOTAL ----------------------- \n");
    printf("\n Acumulated sum TOTAL: %lf",sumaT);
    resultadoT=sqrt( sumaT );
    printf("\n Result TOTA: %lf \n",resultadoT);
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
