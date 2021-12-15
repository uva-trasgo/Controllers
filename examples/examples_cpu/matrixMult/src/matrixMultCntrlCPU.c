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
CAL_KERNEL(MatMul, CPU, KHitTile_double A,  KHitTile_double B, KHitTile_double C){
	int k=0;
	for(k=0; k< hit_tileDimCard(C,1); k++ ){

		hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y) =hit_tileElemAtNoStride(C, 2, threadId.x, threadId.y) +
			 hit_tileElemAtNoStride(A, 2, threadId.x, k) * hit_tileElemAtNoStride(B, 2, k, threadId.y);
	}
}
/* B. Defining kernel prototypes */
CAL_KERNEL_PROTO( MatMul,                                               // Name
                  1, CPU,                                       // Implementations
                  3, IN, HitTile_double, A, IN, HitTile_double, B, OUT, HitTile_double, C       // Arguments
                );


/* C. Function to initialize the matrices */
void initMatrix( HitTile_double matrixA, HitTile_double matrixB, HitTile_double matrixC, int rows, int columns ) {

        /* 1. INIT Mat = 0 */
        int i,j;
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
			hit_tileElemAt( matrixA, 2, i, j ) =18.345; // (i+j)%100;
		}
	}
        for (i=0; i<columns; i++){
                for (j=0; j<rows; j++){
			hit_tileElemAt( matrixB, 2, i, j ) = 1.14; //(i*j)%100;
		}
	}
        for (i=0; i<rows; i++){
                for (j=0; j<rows; j++){
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
	// 1. Init Controllers library
    	CAL_CntrlInit(1);

	// 2. Taking arguments
   	if ( argc != 4 ) {
                fprintf(stderr, "\nUsage: %s <numRows> <numColumns> <numThreads>\n", argv[0]);
                exit(EXIT_FAILURE);
   	 }
   	 int rows = atoi( argv[1] );
   	 int columns = atoi( argv[2] );
   	 int num_threads=atoi( argv[3]);
    
    	int i, j, k;

#ifdef DEBUG
printf("\n Rows: %d Columns: %d \n", rows,columns);
#endif

    	// 3. Declare and initialize full matrices and domains
    	HitTile_double matrixA, matrixB, matrixC;
    	hit_tileDomain( &matrixA, double, 2, rows, columns );
    	hit_tileDomain( &matrixB, double, 2, columns, rows );
    	hit_tileDomain( &matrixC, double, 2, rows, rows );

    	hit_tileAlloc( &matrixA );
    	hit_tileAlloc( &matrixB );
    	hit_tileAlloc( &matrixC );
   	initMatrix( matrixA, matrixB, matrixC, rows, columns );

	// 4. Init computation threads
    	CALThread threads;
    	CALThreadInit(threads, 2, rows, columns);

#ifdef DEBUG2 
	printf("\n Matrix A\n");
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        printf("%lf ", hit_tileElemAt( matrixA, 2, i, j ));
		}
		printf("\n");
	}

	printf("\n Matrix B\n");
        for (i=0; i<columns; i++){
                for (j=0; j<rows; j++){
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
	CAL_CntrlLaunch(comm, MatMul, threads, matrixA, matrixB, matrixC);	
   

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
        #pragma omp for
        for (i=0; i<rows; i++){
                for (j=0; j<rows; j++){
                        printf("%lf ", hit_tileElemAt( matrixC, 2, i, j ));
		}
		printf("\n");
	}
#endif


        // 11. Calculate NORM
        double resultado=0,suma=0;
        for ( i=0; i<rows; i++ ) {
                for ( j=0; j<rows; j++ ) {
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
