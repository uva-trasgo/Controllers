#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>



double  mainClock;
double  initClock;
double  sequentialClock;


/* A. INITIALIZE MATRIX */
void initMatrix( double *matrixA, double *matrixB, double *matrixC, int rows, int columns ) {

        /* 1. INIT Mat = 0 */
        int i,j;
        #pragma omp for private(i,j) collapse(2)
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        matrixA[ i*columns + j ] = 18.345; //(i+j)%100;
		}
	}

        #pragma omp for private(i,j) collapse(2)
        for (i=0; i<columns; i++){
                for (j=0; j<rows; j++){
                        matrixB[ i*rows + j ] = 1.14; //(i*j)%100;
		}
	}

        #pragma omp for private(i,j) collapse(2)
        for (i=0; i<rows; i++){
                for (j=0; j<rows; j++){
                        matrixC[ i*rows + j ] = 0;
		}
	}
}

double t_start, t_end;

int main(int argc, char *argv[])
{


    if ( argc != 4 ) {
                fprintf(stderr, "\nUsage: %s <numRows> <numColumns> <numThreads>\n", argv[0]);
                exit(EXIT_FAILURE);
    }
    int rows = atoi( argv[1] );
    int columns = atoi( argv[2] );
    int num_threads=atoi( argv[3]);
    omp_set_num_threads(num_threads);

    int i, j, k;

#ifdef DEBUG
printf("\n Rows: %d Columns: %d \n", rows,columns);
#endif

    mainClock = omp_get_wtime();

    
    /* 2. DECLARE FULL MATRIX AND COPY */
    double *matrixA, *matrixB, *matrixC;
    matrixA = (double *)malloc( (size_t)rows * (size_t)columns * sizeof(double) );
    matrixB = (double *)malloc( (size_t)columns * (size_t)rows * sizeof(double) );
    matrixC = (double *)malloc( (size_t)rows * (size_t)rows * sizeof(double) );

    if ( matrixA == NULL || matrixB == NULL || matrixC == NULL ) {
                perror("Allocating memory for matrices");
                exit(EXIT_FAILURE);
    }

    #pragma omp parallel private(i,j,k) firstprivate(rows, columns)
    {

    initMatrix( matrixA, matrixB, matrixC, rows, columns );

#ifdef DEBUG 
	printf("\n Matrix A\n");
        #pragma omp for
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        printf("%lf ",matrixA[ i*columns + j ]);
		}
		printf("\n");
	}

	printf("\n Matrix B\n");
        #pragma omp for
        for (i=0; i<columns; i++){
                for (j=0; j<rows; j++){
                        printf("%lf ",matrixB[ i*rows + j ]);
		}
		printf("\n");
	}

#endif

    #pragma omp for
    for(i=0; i<rows; i++)
        for(j=0; j<columns; j++)  
            for(k=0; k<columns; k++)
                matrixC[i*rows +j] = matrixC[i*rows + j] + matrixA[i*columns + k] * matrixB[k*rows + j];

    }

    /* 4. CLOCK RESULTS */
    mainClock = omp_get_wtime() - mainClock;

    // TIMES
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock main: %lf\n", mainClock );



#ifdef DEBUG 
	printf("\n Matrix C\n");
        #pragma omp for
        for (i=0; i<rows; i++){
                for (j=0; j<rows; j++){
                        printf("%lf ",matrixC[ i*rows + j ]);
		}
		printf("\n");
	}
#endif


     // Calculate NORM
     double resultado=0,suma=0;
     for ( i=0; i<rows; i++ ) {
            for ( j=0; j<rows; j++ ) {
              suma += pow(  matrixC[ i*rows + j ] ,2);
            }
    }
    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum: %lf",suma);

    resultado=sqrt( suma );
    printf("\n Result: %lf \n",resultado);

    printf("\n ---------------------------------------------------- \n");

    free(matrixA);
    free(matrixB);
    free(matrixC);
  
    return 0;
}
