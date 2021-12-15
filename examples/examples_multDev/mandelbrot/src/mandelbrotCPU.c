/*
 * Ejemplo: 
 * 		2000 400 400 -0.564495188 -0.557173554 -0.486029079 -0.480537853
 * 		Max: 962
 */
#include<stdio.h>
#include<stdlib.h>
#include<omp.h>

double time;
 
int main(int argc, char *argv[] ) {
	if ( argc != 9 ) {
		fprintf(stderr, "Usage: %s <maxiter> <xsize> <ysize> <begin_x> <end_x> <begin_y> <end_y> <numThreads>\n", argv[0]);
		exit (EXIT_FAILURE);
	}

	int maxiter = atoi( argv[1] );
	int xsize = atoi( argv[2] );
	int ysize = atoi( argv[3] );
	float begin_x = (float)atof( argv[4] );
	float end_x = (float)atof( argv[5] );
	float begin_y = (float)atof( argv[6] );
	float end_y = (float)atof( argv[7] );
	int numThreads = atoi( argv[8] );

	printf("Mandelbrot set: [%f,%f][%f,%f]\n", begin_x, end_x, begin_y, end_y );
	printf("Max.Iterations: %d, Matrix [%d,%d]\n", maxiter, xsize, ysize );

	float *resultx = (float *)malloc( sizeof(float)* xsize * (size_t)ysize );
	float *resulty = (float *)malloc( sizeof(float) * xsize * (size_t)ysize );
	if ( resultx== NULL || resulty== NULL ) {
		fprintf(stderr, "Error allocating memory for result...\n");
		exit (EXIT_FAILURE);
	}

	int i,j;

	time = omp_get_wtime();

	for (i=0; i<xsize; i++){
		for (j=0; j<ysize; j++) {
			float x0 = begin_x + (end_x - begin_x)/xsize * i;
			float y0 = begin_y + (end_y - begin_y)/ysize * j;
			float x = 0;
			float y = 0;
#ifdef DEBUG
	if ( i == 0 && j == 0 ) 
		printf("[0,0] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
	if ( i == 31 && j == 8 ) 
		printf("[31,8] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
	if ( i == 38 && j == 2 ) 
		printf("[38,2] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
	if ( i == 39 && j == 0 ) 
		printf("[39,0] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
#endif
			int maxiter2=2000;
			int iteration = 0;
			while (x*x + y*y < 2*2 && iteration < maxiter ) {
				float xtemp = x*x - y*y + x0;
				y = 2*x*y + y0;
				x = xtemp;
				iteration = iteration + 1;
			}
			//result[ i*ysize + j ] = iteration;
			resultx[ i*ysize + j ] = x;
			resulty[ i*ysize + j ] = y;
			//printf("%d \n",iteration);

#ifdef DEBUG
	if ( i == 0 && j == 0 ) 
		printf("[0,0] Valores: (%20.18lf , %20.18lf)\n", x, y );
	if ( i == 31 && j == 8 ) 
		printf("[31,8] Valores: (%20.18lf , %20.18lf)\n", x, y );
	if ( i == 38 && j == 2 ) 
		printf("[38,2] Valores: (%20.18lf , %20.18lf)\n", x, y );
	if ( i == 39 && j == 0 ) 
		printf("[39,0] Valores: (%20.18lf , %20.18lf)\n", x, y );
#endif

		}}

	time = omp_get_wtime() - time;

#ifdef MAX_VALUE
	int maxvalue = 0;
#endif
	for (i=0; i<xsize; i++) {

		for (j=0; j<ysize; j++) {
#ifdef MAX_VALUE
			if ( result[ i*ysize + j ] > maxvalue ) maxvalue = result[ i*ysize+j ];
#endif
#ifdef WRITE_OUTPUT
			printf("%.8lf,%.8lf  ", resultx[ i*ysize+j ], resulty[ i*ysize+j ] );
#endif
		}
#ifdef WRITE_OUTPUT
		printf("\n");
#endif
	}

#ifdef MAX_VALUE
	printf("Result: %d, %lf\n", maxvalue, time);
#else
	printf("Clock main: %lf\n", time);
#endif


	return 0;
}

