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
#include "CAL_CntrlXPHI.h"

hit_tileNewType( float );
hit_ktileNewType( float );


double  mainClock;
double  initClock;
double  sequentialClock;


/* A. Xeon Phi Kernel implementation */
CAL_KERNEL(MatSum, XPhi, int nIter, KHitTile_float A, KHitTile_float B, KHitTile_float C){
  int k;
  #pragma vector aligned
  #pragma ivdep
  for(k=0;k<nIter;k++){
        hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y) = hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y) + hit_tileElemAtNoStride(A, 2, threadId.x, threadId.y) + hit_tileElemAtNoStride(B, 2, threadId.x, threadId.y) ;
  }
  printf("XPhi: :%lf + %lf =%lf \n", hit_tileElemAtNoStride(A, 2, threadId.x, threadId.y) , hit_tileElemAtNoStride(B, 2, threadId.x, threadId.y) ,hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y));

}


/* B. Generic Kernel implementation */
CAL_KERNEL(MatSum, Generic, int nIter, KHitTile_float A, KHitTile_float B, KHitTile_float C ) {
  int k;
  for(k=0;k<nIter;k++){
        hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y) = hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y) + hit_tileElemAtNoStride(A, 2, threadId.x, threadId.y) + hit_tileElemAtNoStride(B, 2, threadId.x, threadId.y) ;
  }
  printf("Generic: %lf \n", hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y));
}

/* C. Defining kernel prototypes */
CAL_KERNEL_PROTO( MatSum,                                               // Name
                  2, XPhi, Generic,                                      // Implementations
                  4, INVAL, int, nIter, IN, HitTile_float, A, IN, HitTile_float, B, OUT, HitTile_float, C    // Arguments
                );


/* 1. INITIALIZE MATRIX */
void initMatrix( HitTile_float matrixA, HitTile_float matrixB, HitTile_float matrixC, int rows, int columns ) {

        /* 1. INIT Mat = 0 */
        int i,j;
   //     #pragma omp for private(i,j) collapse(2)
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
			hit_tileElemAt( matrixA, 2, i, j ) =0.12; // (i+j)%100;
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


double t_start, t_end;

int main(int argc, char *argv[])
{

//    #ifdef DEBUG
     setbuf(stdout, NULL);
     setbuf(stderr, NULL);
  //  #endif

    CAL_CntrlInit(1);

    if ( argc < 3 ) {
                fprintf(stderr, "\nUsage: %s <numRows> \n", argv[0]);
                exit(EXIT_FAILURE);
    }
    int rows = atoi( argv[1] );
    int nIter = atoi( argv[2] );
    int columns = rows;

    int i, j, k;

#ifdef DEBUG
printf("\n Rows: %d Columns: %d \n", rows,columns);
#endif

#pragma offload target(mic:0)
{
#ifdef DEBUG
	char michostname[100];
  	gethostname(michostname, sizeof(michostname));
  	printf("MIC: Hello world from MIC. I am %s and I have %ld logical cores. \n", michostname, sysconf(_SC_NPROCESSORS_ONLN));
#endif
}
    
    /* 2. DECLARE FULL MATRIX AND COPY */
    HitTile_float matrixA, matrixB, matrixC;
    hit_tileDomain( &matrixA, float, 2, rows, columns );
    hit_tileDomain( &matrixB, float, 2, rows, columns );
    hit_tileDomain( &matrixC, float, 2, rows, columns );

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
    mainClock = omp_get_wtime();
    
    CAL_CntrlCreate(&comm, CAL_CNTRL_XPHI, 228);
    CAL_CntrlAttach(&comm, (HitTile*)&matrixA);
    CAL_CntrlAttach(&comm, (HitTile*)&matrixB);
    CAL_CntrlAttach(&comm, (HitTile*)&matrixC);
  
    CAL_CntrlLaunch(comm, MatSum, threads, nIter, matrixA, matrixB, matrixC);	

    CAL_CntrlDetach(&comm, (HitTile*)&matrixC);

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
