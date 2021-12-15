/*
 * Ejemplo: 
 * 		2000 400 400 -0.564495188 -0.557173554 -0.486029079 -0.480537853
 * 		Max: 962
 */
#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include <mkl.h>
#include <math.h>

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
	#pragma offload target(mic:0) in(xsize, ysize, begin_x,begin_y, end_y, end_x ), \
                                      out(resultx:length(xsize*ysize)), out(resulty:length(xsize*ysize))

      {

	#pragma	omp parallel for private( j ) firstprivate( xsize,ysize, begin_x,begin_y, end_y, end_x )
	for (i=0; i<xsize; i++){
		for (j=0; j<ysize; j++) {
			float x0 = begin_x + (end_x - begin_x)/xsize * i;
			float y0 = begin_y + (end_y - begin_y)/ysize * j;
			float x = 0;
			float y = 0;
			int maxiter2=2000;
			int iteration = 0;
			while (x*x + y*y < 2*2 && iteration < maxiter ) {
				float xtemp = x*x - y*y + x0;
				y = 2*x*y + y0;
				x = xtemp;
				iteration = iteration + 1;
			}
			resultx[ i*ysize + j ] = x;
			resulty[ i*ysize + j ] = y;

		}}
  
       }

	time = omp_get_wtime() - time;

    /* CHECK */
    double resultado=0,suma=0;
    for ( i=0; i<xsize; i++ ) {
         for ( j=0; j<ysize; j++ ) {
              suma += pow(   resultx[ i*ysize+ j ]+ resulty[ i*ysize+ j ]  ,2);
            }
    }
    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum: %lf",suma);

    resultado=sqrt( suma );
    printf("\n Result: %lf \n",resultado);

    printf("\n ---------------------------------------------------- \n");


     printf("Clock main: %lf\n", time);


	return 0;
}

