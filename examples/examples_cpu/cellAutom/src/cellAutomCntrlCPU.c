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

/* A. CPU Kernel implementations */
CAL_KERNEL(Compute, CPU, KHitTile_double src, KHitTile_double dst){

	int row = threadId.x + 1;
	int col = threadId.y + 1;
	hit_tileElemAtNoStride(dst, 2, row, col) = ( hit_tileElemAtNoStride(src, 2, row-1, col) +
					     hit_tileElemAtNoStride(src, 2, row + 1, col    ) +
					     hit_tileElemAtNoStride(src, 2, row    , col - 1) +
					     hit_tileElemAtNoStride(src, 2, row    , col + 1) ) / 4;
}

CAL_KERNEL(Update, CPU, KHitTile_double src, KHitTile_double dst){

	int row = threadId.x ;
	int col = threadId.y ;
	hit_tileElemAtNoStride(dst, 2, row, col) =  hit_tileElemAtNoStride(src, 2, row, col); 

}

/* B. Defining kernel prototypes */
CAL_KERNEL_PROTO( Compute,                                               // Name
                  1, CPU,	                                       // Implementations
                  2, IN, HitTile_double, src, OUT, HitTile_double, dst       // Arguments
                );
CAL_KERNEL_PROTO( Update,                                               // Name
                  1, CPU,	                                       // Implementations
                  2, IN, HitTile_double, src, OUT, HitTile_double, dst       // Arguments
		);


/* C. INITIALIZE MATRIX */
void initMatrix( HitTile_double tileMat ) {

        double zero=0;
        int i,j;
        hit_tileForDimDomain( tileMat, 0, i )
                hit_tileForDimDomain( tileMat, 1, j )
                        hit_tileElemAtNoStride( tileMat, 2, i, j ) = 0;

        hit_tileForDimDomain( tileMat, 1, j ){
                hit_tileElemAtNoStride( tileMat, 2, 0, j ) = 1;
                hit_tileElemAtNoStride( tileMat, 2, hit_tileDimCard( tileMat, 0 )-1, j ) = 2;

        }

        hit_tileForDimDomain( tileMat, 0, i ){
                hit_tileElemAtNoStride( tileMat, 2, i, 0 ) = 3;
                hit_tileElemAtNoStride( tileMat, 2, i, hit_tileDimCard( tileMat, 1 )-1 ) = 4;

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
                fprintf(stderr, "\nUsage: %s <numRows> <iterations> <numThreads>\n", argv[0]);
                exit(EXIT_FAILURE);
    }
    int rows = atoi( argv[1] );
    int columns = rows;
    int nIter = atoi( argv[2] );
    int num_threads=atoi( argv[3]);
    
    int i, j, k;

#ifdef DEBUG
    setbuf(stderr, NULL);
    setbuf(stdout, NULL);
    printf("\n Rows: %d Columns: %d \n", rows,columns);
#endif


    // 3. Declare and initialize full matrices and domains
    HitTile_double matrix, matrixCopy;
    hit_tileDomain( &matrix, double, 2, rows, columns );
    hit_tileDomain( &matrixCopy, double, 2, rows, columns );

    hit_tileAlloc( &matrix );
    hit_tileAlloc( &matrixCopy );
    initMatrix( matrix );
    initMatrix( matrixCopy );
    
    // 4. Init computation threads
    CALThread threads;
    CALThread threadsUpdate;
    CALThreadInit(threadsUpdate, 2, rows, columns);
    CALThreadInit(threads, 2, rows-2, columns-2);



#ifdef DEBUG 
	printf("\n Matrix \n");
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        printf("%lf ", hit_tileElemAtNoStride( matrix, 2, i, j ));
		}
		printf("\n");
	}

#endif

    double tmpClock;
    mainClock = omp_get_wtime();

    // 5. Create controller object
    CALCntrl comm;
    CAL_CntrlCreate(&comm, CAL_CNTRL_CPU, num_threads);

    // 6. Attach data structures to the controllers
    CAL_CntrlAttach(&comm, (HitTile*)&matrix);
    CAL_CntrlAttach(&comm, (HitTile*)&matrixCopy);


    // 7. Launch tasks
    int loopIndex;
    for (loopIndex = 0; loopIndex < nIter ; loopIndex++) {
 
	tmpClock = omp_get_wtime();

	// 7.1 Copy result from device memory to host memory
	CAL_CntrlLaunch(comm, Update, threadsUpdate, matrix, matrixCopy);	
   	
	// 7.2 Second task Compute tile
	CAL_CntrlLaunch(comm, Compute, threads, matrixCopy, matrix);	

        sequentialClock = omp_get_wtime() - tmpClock + sequentialClock;
  
     }

   	
    // 8. Copy result from device memory to host memory
    CAL_CntrlDetach(&comm, (HitTile*)&matrix);
    CAL_CntrlDetach(&comm, (HitTile*)&matrixCopy);

    // 9. Destroy the controller
    CAL_CntrlDestroy(&comm);

    mainClock = omp_get_wtime() - mainClock;

    // 10. TIMES
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock main: %lf\n", mainClock );
    printf("Clock seq: %lf\n", sequentialClock );



#ifdef DEBUG 
	printf("\n Matrix \n");
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        printf("%lf ", hit_tileElemAtNoStride( matrix, 2, i, j ));
		}
		printf("\n");
	}
#endif


    // 11. Calculate NORM
    double resultado=0,suma=0;
    for ( i=1; i<rows-1; i++ ) {
            for ( j=1; j<columns-1; j++ ) {
                suma += pow(  hit_tileElemAtNoStride( matrix, 2, i, j ) ,2);
            }
    }
    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum: %lf",suma);

    resultado=sqrt( suma );
    printf("\n Result: %lf \n",resultado);

    printf("\n ---------------------------------------------------- \n");
    
    // 12. Free data structures
    hit_tileFree(matrix);
    hit_tileFree(matrixCopy);

    // 13. Finish the controller library
    CAL_CntrlFinish();

 
    return 0;
}
