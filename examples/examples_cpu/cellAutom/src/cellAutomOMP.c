#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

double	mainClock;
double	initClock;
double	sequentialClock;


/* A. INITIALIZE MATRIX */
void initMatrix( double *tileMat, int size0, int size1 ) {

	/* 1. INIT Mat = 0 */
	int i,j;
	#pragma omp for private(i,j) collapse(2)
	for (i=0; i<size0; i++)
		for (j=0; j<size1; j++)
			tileMat[ i*size1 + j ] = 0;

	/* 2. INIT BORDERS  UP(i)=1, DOWN(i)=2, LEFT(i)=3, RIGHT(i)=4 */
	/* 2.1. FIRST/LAST ROWS */
	#pragma omp for private(j)
	for (j=0; j<size1; j++) {
		tileMat[ j ] = 1;
		tileMat[ (size0-1)*size1 + j ] = 2;
	}
		
	/* 2.3. FIRST/LAST COLUMNS */
	#pragma omp for private(i)
	for (i=0; i<size0; i++) {
		tileMat[ i*size1 ] = 3;
		tileMat[ i*size1 + size1-1 ] = 4;
	}
}


/* B. MAIN: CELLULAR AUTOMATA */
int main(int argc, char *argv[]) {

	int rows, columns, numIter;

	if ( argc != 5 ) {
		fprintf(stderr, "\nUsage: %s <numRows> <numColumns> <numIterations> <numThreads>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	rows = atoi( argv[1] );
	columns = atoi( argv[2] );
	numIter = atoi( argv[3] );

	// 0. FIX NUMBER OF THREADS USING A PROGRAM ARGUMENT
	int num_threads = atoi( argv[4] );
	omp_set_num_threads(num_threads);

	/* 1. INIT CLOCKS */
	initClock = omp_get_wtime();
	sequentialClock = 0;

	/* 2. DECLARE FULL MATRIX AND COPY */
	double *matrix, *copy;
	matrix = (double *)malloc( (size_t)rows * (size_t)columns * sizeof(double) );
	copy = (double *)malloc( (size_t)rows * (size_t)columns * sizeof(double) );
	if ( matrix == NULL || copy == NULL ) {
		perror("Allocating memory for matrices");
		exit(EXIT_FAILURE);
	}

	/* 3. PARALLEL REGION */
	int i,j;
	double c_init=0, c_seq=0;
	int flags = 1;
	/* 3.1. INITIALIZE MATRIX (IN PARALLEL) */
	initMatrix( matrix, rows, columns );

	// CLOCK
	initClock = omp_get_wtime() - initClock;
	if(initClock > c_init) c_init = initClock;

	mainClock = omp_get_wtime();
	#pragma omp parallel private(i,j) firstprivate(initClock,sequentialClock) 
	{
		/* 3.2. COMPUTATION LOOP */
		int loopIndex;
		for (loopIndex = 0; loopIndex < numIter ; loopIndex++) {
			// CLOCK: Measure secuential time 
			double tmpClock = omp_get_wtime();

			/* 3.2.1. UPDATE TILE COPY */
			#pragma omp for private(j) //collapse(2)
			for ( i=0; i<rows; i++ )
				for ( j=0; j<columns; j++ )
					copy[ i*columns + j ] = matrix[ i*columns + j ];

			/* 3.2.2. COMPUTATION LOOP */
			#pragma omp for private(j)  //collapse(2) 
			for ( i=1; i<rows-1; i++ )
				for ( j=1; j<columns-1; j++ ) {
					matrix[ i*columns + j ] = (
						copy[ (i-1)*columns + j ] +
						copy[ (i+1)*columns + j ] +
						copy[ i*columns + j-1 ] +
						copy[ i*columns + j+1 ] ) / 4;
				}
			// CLOCK
			sequentialClock = omp_get_wtime() - tmpClock + sequentialClock;
		}

		// CLOCK
		#pragma omp critical
		if(sequentialClock > c_seq) c_seq = sequentialClock;
		
	 }
	sequentialClock = c_seq;
	initClock = c_init;

	/* 4. CLOCK RESULTS */
	mainClock = omp_get_wtime() - mainClock;

#ifdef DEBUG 
        printf("\n Matrix \n");
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        printf("%lf ",  matrix[ i*columns+ j ]);
                }
                printf("\n");
        }
#endif

	// TIMES
        printf("\n ----------------------- TIME ----------------------- \n");
	printf("Clock main: %lf\n", mainClock );
	printf("Clock init: %lf\n", initClock );
	printf("Clock sequential: %lf\n", sequentialClock );

	// Calculate NORM
	double resultado=0,suma=0;
        for ( i=1; i<rows-1; i++ ) {
                for ( j=1; j<columns-1; j++ ) {
                  suma += pow(  matrix[ i*columns + j ] ,2);
		}
        }
	printf("\n ----------------------- NORM ----------------------- \n");
	printf("\n Acumulated sum: %lf",suma);

        resultado=sqrt( suma );
        printf("\n Result: %lf \n",resultado);

	printf("\n ---------------------------------------------------- \n");

	/* 5. WRITE RESULT MATRIX */
#ifdef WRITE_RESULT
	FILE *output;
	output = fopen("Result.out.dtxt", "w");
	if ( output == NULL ) {
		perror("Opening output file");
		exit(EXIT_FAILURE);
	}
	for ( i=0; i<rows; i++ ) {
		for ( j=0; j<columns; j++ ) {
			fprintf( output, "%14.2lf ", matrix[ i*columns + j ] );
		}
	}
#endif

	/* 6. FREE RESOURCES */
	free( matrix );
	free( copy );
	return 0;
}
