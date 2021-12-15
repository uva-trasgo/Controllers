#define _BSD_SOURCE
#include <omp.h>
#include <hitmap.h>
#include <math.h>
//#include <mkl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "CAL.h"
#include "CAL_KernelXPHI.h"
#include "CAL_AttachXPHI.h"

hit_tileNewType( double );


double  mainClock;
double  initClock;
double  sequentialClock;

#define SIZE 16

CAL_KERNEL_XPHI(MatSum, 3, IN, HitTile_double*, A, IN, HitTile_double*, B, OUT, HitTile_double*, C){
  int k;

  int nIter=100;

  #pragma vector aligned
  #pragma ivdep
  for(k=0;k< nIter;k++){
	hit_tileElemAt(*C, 2, threadId.x, threadId.y) = hit_tileElemAt(*C, 2, threadId.x, threadId.y) + hit_tileElemAt(*A, 2, threadId.x, threadId.y) + hit_tileElemAt(*B, 2, threadId.x, threadId.y) ;
  }
 printf("%lf\n ",hit_tileElemAt(*C, 2, threadId.x, threadId.y));
}


/* A. INITIALIZE MATRIX */
void initMatrix( HitTile_double matrixA, HitTile_double matrixB, HitTile_double matrixC, int rows, int columns ) {

        /* 1. INIT Mat = 0 */
        int i,j;
   //     #pragma omp for private(i,j) collapse(2)
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
			hit_tileElemAt( matrixA, 2, i, j ) =2.12; // (i+j)%100;
		}
	}

     //   #pragma omp for private(i,j) collapse(2)
        for (i=0; i<columns; i++){
                for (j=0; j<rows; j++){
			hit_tileElemAt( matrixB, 2, i, j ) = 0.2; //(i*j)%100;
		}
	}

       // #pragma omp for private(i,j) collapse(2)
        for (i=0; i<rows; i++){
                for (j=0; j<rows; j++){
                        //matrixC[ i*rows + j ] = 0;
			hit_tileElemAt( matrixC, 2, i, j ) = 0;
		}
	}
}



CAL_KERNEL_XPHI(Pru, 1, IN, HitTile_double*, A ){
	printf("LAUNCH: MAtriz A: %d\n", hit_tileDims(*A));
	printf("%lf\n", hit_tileElemAt(*A, 2, threadId.x, threadId.y));
        char michostname[100];
        gethostname(michostname, sizeof(michostname));
        printf("LAUNCH: MIC: Hello world from MIC. I am %s and I have %ld logical cores. I was called from host: CHIMERA \n", michostname, sysconf(_SC_NPROCESSORS_ONLN));
}


/************************************/
/* Estructura de un launch */
/************************************/
void prueba(HitTile_double *mA){

   HitTile_double vf=*mA;
   double *data_2= vf.data;
   #pragma offload target(mic:0) in(vf) in(data_2:length(0) alloc_if(0) free_if(0)) 
   {
	HitTile_double *Z=&vf;
        vf.data=data_2;

//	printf("EEEEEEEE MAtriz A: %d\n", hit_tileDims(*Z));
//	printf("%lf\n", hit_tileElemAt(*Z, 2, 2, 2));
//	printf("%lf\n", data_2[0]);
        char michostname[100];
        gethostname(michostname, sizeof(michostname));
        printf("MIC: Hello world from MIC. I am %s and I have %ld logical cores. I was called from host: CHIMERA \n", michostname, sysconf(_SC_NPROCESSORS_ONLN));
 }
}
/************************************/
void prueba_out(HitTile_double *mA){

   HitTile_double vf=*mA;
   double *data_2= vf.data;
   #pragma offload target(mic:0) in(vf) in(data_2:length(0) alloc_if(0) free_if(0)) 
   {
	HitTile_double *Z=&vf;
        vf.data=data_2;

	printf("EEEEEEEE MAtriz A: %d\n", hit_tileDims(*Z));
	printf("%lf\n", hit_tileElemAt(*Z, 2, 2, 2));
	printf("%lf\n", data_2[0]);
        char michostname[100];
        gethostname(michostname, sizeof(michostname));
        printf("MIC: Hello world from MIC. I am %s and I have %ld logical cores. I was called from host: CHIMERA \n", michostname, sysconf(_SC_NPROCESSORS_ONLN));
 }
}
/************************************/


double t_start, t_end;

int main(int argc, char *argv[])
{

setbuf(stdout, NULL);
setbuf(stderr, NULL);

    CAL_CntrlInit(1);

    if ( argc != 4 ) {
                fprintf(stderr, "\nUsage: %s <numRows> <numColumns> <numThreads>\n", argv[0]);
                exit(EXIT_FAILURE);
    }
    int rows = atoi( argv[1] );
    int columns = rows;
    int nIter = atoi( argv[2] );
    int num_threads=atoi( argv[3]);

    int i, j, k;

#ifdef DEBUG
printf("\n Rows: %d Columns: %d \n", rows,columns);
#endif


    
    /* 2. DECLARE FULL MATRIX AND COPY */
    HitTile_double matrixA, matrixB, matrixC;
    hit_tileDomain( &matrixA, double, 2, rows, columns );
    hit_tileDomain( &matrixB, double, 2, rows, columns );
    hit_tileDomain( &matrixC, double, 2, rows, columns );

    hit_tileAlloc( &matrixA );
    hit_tileAlloc( &matrixB );
    hit_tileAlloc( &matrixC );

    CALCntrl comm;
    CALThread threads;

    CALThreadInit(threads, 2, rows, columns);


    initMatrix( matrixA, matrixB, matrixC, rows, columns );

#ifdef DEBUG2 
	printf("\n Matrix A\n");
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        printf("%lf ", hit_tileElemAt( matrixA, 2, i, j ));
		}
		printf("\n");
	}

	printf("\n Matrix B\n");
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        printf("%lf ", hit_tileElemAt( matrixB, 2, i, j ));
		}
		printf("\n");
	}

#endif


    /* CREATE CNTRLS */
    CAL_CntrlCreate(&comm, CAL_CNTRL_XPHI, 228);
    //attachingToXPHI(&(comm.impl.xphi), (HitTile*)&matrixA);
    //attachingToXPHI(&(comm.impl.xphi), (HitTile*)&matrixB);
    //attachingToXPHI(&(comm.impl.xphi), (HitTile*)&matrixC);


    ATTACH(comm, matrixA);
    ATTACH(comm, matrixB);
    ATTACH(comm, matrixC);
 /*   CAL_CntrlAttach(&comm, (HitTile*)&matrixA);
    CAL_CntrlAttach(&comm, (HitTile*)&matrixB);
    CAL_CntrlAttach(&comm, (HitTile*)&matrixC);
   */
/************************************/
/* Estructura de un Attach          */
/************************************/

   double *data = matrixA.data;
  int numElems;
HitShape tile_m = hit_tileShape(matrixC);
  if(hit_shapeDims(tile_m)==1){
    numElems=hit_shapeSigCard(tile_m,0) ;
    } 
  if(hit_shapeDims(tile_m)==2){
    numElems=hit_shapeSigCard(tile_m,0)*hit_shapeSigCard(tile_m,1); 
    } 
  if(hit_shapeDims(tile_m)==3){
    numElems=hit_shapeSigCard(tile_m,0)*hit_shapeSigCard(tile_m,1)*hit_shapeSigCard(tile_m,2); 
    } 
  if(hit_shapeDims(tile_m)==4){
    numElems=hit_shapeSigCard(tile_m,0)*hit_shapeSigCard(tile_m,1)*hit_shapeSigCard(tile_m,2)*hit_shapeSigCard(tile_m,3); 
    } 

/*
   #pragma offload target(mic:0) in(matrixA) \
                 in(data:length(numElems) alloc_if(1) free_if(0)) 
   {
      matrixA.data=data;

	printf("MAtriz A: %d\n", hit_tileDims(matrixA));
	printf("%lf\n", hit_tileElemAt(matrixA, 2, 2, 2));
        char michostname[100];
        gethostname(michostname, sizeof(michostname));
        printf("MIC: Hello world from MIC. I am %s and I have %ld logical cores. I was called from host: CHIMERA \n", michostname, sysconf(_SC_NPROCESSORS_ONLN));
   }
*/
 
/************************************/

    mainClock = omp_get_wtime();
    CAL_CntrlLaunch(comm, MatSum, threads, 3, &matrixA, &matrixB, &matrixC);	

    //CAL_CntrlLaunch(comm, Pru, threads, 1, &matrixA);	
    //CAL_CntrlLaunch(comm, MatSum, threads, 3, &matrixA, &matrixB, &matrixC);	
	
   // prueba_out(&matrixA);
//sleep(5);
    // Copy result from device memory to host memory
    //CAL_CntrlDetach(&comm, (HitTile*)&matrixA);
//    CAL_CntrlDetach(&comm, (HitTile*)&matrixB);
    CAL_CntrlDetach(&comm, (HitTile*)&matrixC);

/*
	double *data_3=matrixC.data; 
	#pragma offload target(mic:0) in(data_3:length(0) alloc_if(0) free_if(0)) out(data_3:length(numElems) alloc_if(0) free_if(1))
	{

	}
*/  

  CAL_CntrlDestroy(&comm);


    /* 4. CLOCK RESULTS */
    mainClock = omp_get_wtime() - mainClock;

    // TIMES
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock main: %lf\n", mainClock );



#ifdef DEBUG2 
	printf("\n Matrix C\n");
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        printf("%lf ", hit_tileElemAt( matrixC, 2, i, j ));
		}
		printf("\n");
	}
#endif


    // Calculate NORM
    double resultado=0,suma=0;
    for ( i=0; i<rows; i++ ) {
         for ( j=0; j<columns; j++ ) {
              suma += pow(  hit_tileElemAt( matrixC, 2, i, j ) ,2);
            }
    }
    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum: %lf",suma);

    resultado=sqrt( suma );
    printf("\n Result: %lf \n",resultado);

    printf("\n ---------------------------------------------------- \n");

    hit_tileFree(matrixA);
    hit_tileFree(matrixB);
    hit_tileFree(matrixC);

    CAL_CntrlFinish();  

  return 0;
}
