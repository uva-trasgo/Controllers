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

hit_tileNewType( float );
//hit_xtileNewType( doubile );
/*
static void  __attribute__((target(mic))) hit_xtile(HitXTile *result, HitTile *ref, float *data){
//        HitTile         *ref = (HitTile *)refP;
        (*result).data= (float*) data;
        (*result).origAcumCard[0] = (*ref).origAcumCard[0];
        (*result).origAcumCard[1] = (*ref).origAcumCard[1];
        (*result).origAcumCard[2] = (*ref).origAcumCard[2];
        (*result).origAcumCard[3] = (*ref).origAcumCard[3];
        (*result).card[0]= (*ref).card[0];
        (*result).card[1]= (*ref).card[1];
        (*result).card[2]= (*ref).card[2];
        (*result).qstride[0]=1;// (*ref).qstride[0];
        (*result).qstride[1]=1; // (*ref).qstride[1];
        (*result).qstride[2]=1;// (*ref).qstride[2];
}
*/
double  mainClock;
double  initClock;
double  sequentialClock;


CAL_KERNEL_XPHI(MatSum, 3, IN, HitTile_float*, A, IN, HitTile_float*, B, OUT, HitTile_float*, C){
	#ifdef DEBUG
		printf("qStride: %d %d %d\n", C.qstride[0], C.qstride[1], C.qstride[2]);
		printf("qAcum: %d %d %d\n", C.origAcumCard[0], C.origAcumCard[1], C.origAcumCard[2]);
	#endif
        
	int k=0;
  	//#pragma vector aligned
  	#pragma ivdep
        for(k=0; k< hit_tileDimCard(C,1); k++ ){
                hit_tileElemAtNoStride(C,2, threadId.x, k) =hit_tileElemAtNoStride(C, 2, threadId.x, k) +
                         hit_tileElemAtNoStride(A, 2, threadId.x, threadId.y) * hit_tileElemAtNoStride(B, 2, threadId.y, k);
        }



}


/* A. INITIALIZE MATRIX */
void initMatrix( HitTile_float matrixA, HitTile_float matrixB, HitTile_float matrixC, int rows, int columns ) {

        /* 1. INIT Mat = 0 */
        int i,j;
   //     #pragma omp for private(i,j) collapse(2)
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
			hit_tileElemAt( matrixA, 2, i, j ) = (i+j)%100;
		}
	}

     //   #pragma omp for private(i,j) collapse(2)
        for (i=0; i<columns; i++){
                for (j=0; j<rows; j++){
			hit_tileElemAt( matrixB, 2, i, j ) = (i-j)%100;
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


float t_start, t_end;

int main(int argc, char *argv[])
{

    #ifdef DEBUG
     setbuf(stdout, NULL);
     setbuf(stderr, NULL);
    #endif

    CAL_CntrlInit(1);

    if ( argc< 2 ) {
                fprintf(stderr, "\nUsage: %s <numRows> ", argv[0]);
                exit(EXIT_FAILURE);
    }
    int rows = atoi( argv[1] );
    int columns = rows;

    int i, j, k;

#ifdef DEBUG
printf("\n Rows: %d Columns: %d \n", rows,columns);
#endif

    
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
   
    CAL_CntrlLaunch(comm, MatSum, threads, 3, &matrixA, &matrixB, &matrixC);	
    
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
