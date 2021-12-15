/*
 * Ejemplo: 
 * 		2000 400 400 -0.564495188 -0.557173554 -0.486029079 -0.480537853
 * 		Max: 962
 */
#include<stdio.h>
#include<stdlib.h>
#include <math.h>
#include<sys/time.h>
#include<cuda.h>

double wtime() {
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return (double)tv.tv_sec + (double)tv.tv_usec / 1000000;
}


__global__ void mandelbrotGPU( float *resultx,  int xsize, int ysize, 
					float begin_x, float end_x,
					float begin_y, float end_y,
					int maxiter ) {
	int i = blockDim.y * blockIdx.y + threadIdx.y;
	int j = blockDim.x * blockIdx.x + threadIdx.x;

	if ( i >= xsize || j >= ysize ) return;

	float x0 = begin_x + (end_x - begin_x)/xsize * i;
	float y0 = begin_y + (end_y - begin_y)/ysize * j;
	float x = 0;
	float y = 0;
	int maxiter2=60000;
#ifdef DEBUG
	if ( i == 0 && j == 0 ) 
		printf("[0,0] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
	if ( i == 31 && j == 8 ) 
		printf("[31,8] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
	if ( i == 39 && j == 0 ) 
		printf("[39,0] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
	if ( i == 0 && j == 39 ) 
		printf("[0,39] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
	if ( i == 38 && j == 2 ) 
		printf("[38,2] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
#endif

	int iteration = 0;
	while (x*x + y*y < 2*2 && iteration < maxiter2 ) {
		float xtemp = x*x - y*y + x0;
		y = 2*x*y + y0;
		x = xtemp;
		iteration = iteration + 1;
	}

#ifdef DEBUG
	if ( i == 0 && j == 0 ) 
		printf("[0,0] Valores: (%20.18lf , %20.18lf)\n", x, y );
	if ( i == 31 && j == 8 ) 
		printf("[31,8] Valores: (%20.18lf , %20.18lf)\n", x, y );
	if ( i == 39 && j == 0 ) 
		printf("[39,0] Valores: (%20.18lf , %20.18lf)\n", x, y );
	if ( i == 0 && j == 39 ) 
		printf("[0,39] Valores: (%20.18lf , %20.18lf)\n", x, y );
	if ( i == 38 && j == 2 ) 
		printf("[38,2] Valores: (%20.18lf , %20.18lf)\n", x, y );
#endif

	resultx[ i*ysize + j ] = (float) iteration;
}


int main(int argc, char *argv[] ) {

        if ( argc < 9 ) {
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

#ifdef DEBUG
	printf("Mandelbrot set: [%f,%f][%f,%f]\n", begin_x, end_x, begin_y, end_y );
	printf("Max.Iterations: %d, Matrix [%d,%d]\n", maxiter, xsize, ysize );
#endif

	float *resultx = (float *)malloc( sizeof(float) * (size_t)xsize * (size_t)ysize );
	if ( resultx == NULL ) {
		fprintf(stderr, "Error allocating memory on the HOST for result...\n");
		exit (EXIT_FAILURE);
	}


	int i,j;

	float *dresultx;
	cudaError_t error;

	error = cudaSetDevice( 1 );
	if ( error != cudaSuccess ) {
		fprintf(stderr, "Error selecting device number: %d\n", 0 );
		exit (EXIT_FAILURE);
	}
	cudaDeviceSynchronize();

	error = cudaMalloc( &dresultx, sizeof(float) * (size_t)xsize * (size_t)ysize );
	if ( error != cudaSuccess ) {
		fprintf(stderr, "Error allocating memory on the DEVICE for result...\n");
		exit (EXIT_FAILURE);
	}
	


	double time2;
	double time3;
#define DIV_UP(a,b)	(((a) + (b) -1 )/(b))	

#ifdef DEBUG
	printf("Grid [%d,%d]\n", DIV_UP(xsize,32), DIV_UP(ysize,32) );
#endif 

	dim3 gridSize = dim3( DIV_UP(ysize,32), DIV_UP(xsize,32) );
	dim3 blockSize = dim3( 32,32 );

	error = cudaSetDevice( 1 );
	cudaDeviceSynchronize();
	double time = wtime();
	mandelbrotGPU<<< gridSize, blockSize >>>( dresultx, xsize, ysize, begin_x, end_x,
						begin_y, end_y, maxiter );
	
	error = cudaSetDevice( 1 );
	cudaDeviceSynchronize();
	double aux= wtime();
	error = cudaGetLastError();
	if ( error != cudaSuccess ) {
		fprintf(stderr, "Error launching the kernel...\n");
		exit (EXIT_FAILURE);
	}

	time2 = aux - time;

	error = cudaMemcpy( resultx, dresultx, sizeof(float) * (size_t)xsize * (size_t)ysize, cudaMemcpyDeviceToHost );
	if ( error != cudaSuccess ) {
		fprintf(stderr, "Error transfering memory from DEVICE to HOST for result...\n");
		exit (EXIT_FAILURE);
	}
	
	
	cudaDeviceSynchronize();
	time = wtime() - time;
	time3= wtime() - aux; 
	// TIMES
	printf("\n ----------------------- TIME ----------------------- \n");
    	printf("Clock comm: %lf\n", time3 );
    	printf("Clock seq: %lf\n", time2 );
    	printf("Clock main: %lf\n", time );


    	double resultado=0,suma=0;
    	for ( i=0; i<xsize; i++ ) {
       	  for ( j=0; j<ysize; j++ ) {
              suma += pow(   (resultx[i*ysize + j] ), 2 );
            }
    	}
    	printf("\n ----------------------- NORM ----------------------- \n");
    	printf("\n Acumulated sum: %lf",suma);

    	resultado=sqrt( suma );
    	printf("\n Result: %lf \n",resultado);

    	printf("\n ---------------------------------------------------- \n");


	return 0;
}

