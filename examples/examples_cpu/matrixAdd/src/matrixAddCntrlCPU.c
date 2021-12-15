#include <stdio.h>
#include <omp.h>
#include <hitmap.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "CAL.h"

hit_tileNewType( double );
hit_ktileNewType( double );


double  mainClock;
double  initClock;
double  sequentialClock;



/* A. CPU Kernel implementation */
CAL_KERNEL(MatSum, CPU, int nIter, KHitTile_double A, KHitTile_double B, KHitTile_double C){
  int k;
  for(k=0;k<nIter;k++){
	hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y) = hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y) + hit_tileElemAtNoStride(A, 2, threadId.x, threadId.y) + hit_tileElemAtNoStride(B, 2, threadId.x, threadId.y) ;
  }
  printf("CPU: :%lf + %lf =%lf \n", hit_tileElemAtNoStride(A, 2, threadId.x, threadId.y) , hit_tileElemAtNoStride(B, 2, threadId.x, threadId.y) ,hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y));
}


/* B. Generic Kernel implementation */
CAL_KERNEL(MatSum, Generic, int nIter, KHitTile_double A, KHitTile_double B, KHitTile_double C ) {
  int k;
  for(k=0;k<nIter;k++){
	hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y) = hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y) + hit_tileElemAtNoStride(A, 2, threadId.x, threadId.y) + hit_tileElemAtNoStride(B, 2, threadId.x, threadId.y) ;
  }
  printf("Generic: %lf \n", hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y));
}

/* C. Defining kernel prototypes */ 
CAL_KERNEL_PROTO( MatSum,      						// Name
                  2, CPU, Generic,        				// Implementations
                  4, INVAL, int, nIter, IN, HitTile_double, A, IN, HitTile_double, B, OUT, HitTile_double, C 	// Arguments
                );


/* D. Function to initialize the matrices */
void initMatrix( HitTile_double matrixA, HitTile_double matrixB, HitTile_double matrixC, int size ) {

        /* 1. INIT Mat = 0 */
        int i,j;
        for (i=0; i<size; i++){
                for (j=0; j<size; j++){
			hit_tileElemAt( matrixA, 2, i, j ) =2.12; // (i+j)%100;
		}
	}

        for (i=0; i<size; i++){
                for (j=0; j<size; j++){
			hit_tileElemAt( matrixB, 2, i, j ) = 0.2; //(i*j)%100;
		}
	}

        for (i=0; i<size; i++){
                for (j=0; j<size; j++){
                        //matrixC[ i*size + j ] = 0;
			hit_tileElemAt( matrixC, 2, i, j ) = 0;
		}
	}
}

double t_start, t_end;


/*
 * Main program to perform matrix addition
 */
int main(int argc, char *argv[])
{
        setbuf(stderr, NULL);
        setbuf(stdout, NULL);

    // 1. Init Controllers library
    CAL_CntrlInit(2);
    
    // 2. Taking arguments
    if ( argc != 4 ) {
                fprintf(stderr, "\nUsage: %s <numRows> <numIter> <numThreads>\n", argv[0]);
                exit(EXIT_FAILURE);
    }
    int size = atoi( argv[1] );
    int nIter = atoi( argv[2] );
    int num_threads=atoi( argv[3]);

    int i, j, k;

#ifdef DEBUG
printf("\n Rows: %d Columns: %d \n", size,size);
#endif

   
    
    // 3. Declare and initialize full matrices and domains
    HitTile_double matrixA, matrixB, matrixC;
    hit_tileDomain( &matrixA, double, 2, size, size );
    hit_tileDomain( &matrixB, double, 2, size, size );
    hit_tileDomain( &matrixC, double, 2, size, size );

    hit_tileAlloc( &matrixA );
    hit_tileAlloc( &matrixB );
    hit_tileAlloc( &matrixC );

    initMatrix( matrixA, matrixB, matrixC, size );
    
    // 4. Init computation threads 
    CALThread threads;
    CALThreadInit(threads, 2, size, size);

#ifdef DEBUG2 
	printf("Pointers:\n matrix A: %p \n matrix B: %p\n matrix C: %p\n", matrixA.data, matrixB.data , matrixC.data );
	
	printf("\n Matrix A\n");
        for (i=0; i<size; i++){
                for (j=0; j<size; j++){
                        printf("%lf ", hit_tileElemAt( matrixA, 2, i, j ));
		}
		printf("\n");
	}

	printf("\n Matrix B\n");
        for (i=0; i<size; i++){
                for (j=0; j<size; j++){
                        printf("%lf ", hit_tileElemAt( matrixB, 2, i, j ));
		}
		printf("\n");
	}
#endif

    mainClock = omp_get_wtime();

    // 5. Create controller object
    CALCntrl comm;
    CAL_CntrlCreate(&comm, CAL_CNTRL_CPU, num_threads);
    
    // 6. Attach data structures to the controllers
    CAL_CntrlAttach(&comm, (HitTile*)&matrixA);
    CAL_CntrlAttach(&comm, (HitTile*)&matrixB);
    CAL_CntrlAttach(&comm, (HitTile*)&matrixC);

    // 7. Launch the task
    CAL_CntrlLaunch(comm, MatSum, threads, nIter, matrixA, matrixB, matrixC);	

    // 8. Copy result from device memory to host memory
    CAL_CntrlDetach(&comm, (HitTile*)&matrixA);
    CAL_CntrlDetach(&comm, (HitTile*)&matrixB);
    CAL_CntrlDetach(&comm, (HitTile*)&matrixC);
   
    // 9. Destroy the controller
    CAL_CntrlDestroy(&comm);


    // 10. TIMES
    mainClock = omp_get_wtime() - mainClock;
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock main: %lf\n", mainClock );


#ifdef DEBUG2 
	printf("\n Matrix C\n");
        for (i=0; i<size; i++){
                for (j=0; j<size; j++){
                        printf("%lf ", hit_tileElemAt( matrixC, 2, i, j ));
		}
		printf("\n");
	}
#endif


    // 11. Calculate NORM
    double resultado=0,suma=0;
    for ( i=0; i<size; i++ ) {
         for ( j=0; j<size; j++ ) {
              suma += pow(  hit_tileElemAt( matrixC, 2, i, j ) ,2);
            }
    }
    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum: %lf",suma);

    resultado=sqrt( suma );
    printf("\n Result: %lf \n",resultado);

    printf("\n ---------------------------------------------------- \n");

    // 12. Free data structures
    hit_tileFree(matrixA);
    hit_tileFree(matrixB);
    hit_tileFree(matrixC);

    // 13. Finish the controller library
    CAL_CntrlFinish();  

  return 0;
}
