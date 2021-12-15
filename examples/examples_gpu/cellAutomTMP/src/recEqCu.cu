#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include "timer.h"
#include "CAL.h"

__global__ void
kernel_recEq(float *C, float *A, float *B, int size, int iter){

	int fila = blockIdx.y*blockDim.y +threadIdx.y;
	int columna = blockIdx.x*blockDim.x +threadIdx.x;
	float value = 0.000025;
	//if(fila <= size - 1 && columna <= size -1){	
		for(int i=0; i<iter; i++)
			C[fila*size + columna] = C[fila*size + columna] + (A[fila*size + columna]+ B[fila*size + columna])*value;
	//}
}//kernel_recEq


/*
 * Main program to perform matrix multiplication
 */
int main(int argc, char *argv[]) {

	if ( argc!=6 ) {
		fprintf(stderr, "Usage: %s <matrixSize> <matrixSize> <iter> <GPU> <Version>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	
    Timer tTotal, tTotalA, tTotalC, tComp, tComm;
    TimerCreate( tTotal ); TimerCreate( tTotalA ); tTotalC;
    TimerCreate( tComp );
    TimerCreate( tComm );

    

	/*Entradas*/
	int SIZE	= atoi( argv[1] );
	int ITER	= atoi( argv[3] );
	int GPU		= atoi( argv[4] );
	int VERSION	= atoi( argv[5] );
	unsigned int memoria = SIZE*SIZE*sizeof(float);
	printf("Tamaño %d x %d,  Iteraciones %d,  GPU %d,  Version %d \n", SIZE, SIZE, ITER, GPU, VERSION);		

	cudaSetDevice(GPU);
	
	dim3 threads (32,32);
	dim3 grid( (int)ceil((float)SIZE/(float)threads.x) , (int)ceil((float)SIZE/(float)threads.y )); 
	
	/*Declaración y reserva de memoria*/
	float *C1 = (float *) malloc(memoria); float *C2 = (float *) malloc(memoria);
	float *A1 = (float *) malloc(memoria); float *A2 = (float *) malloc(memoria);
	float *B1 = (float *) malloc(memoria); float *B2 = (float *) malloc(memoria);
	
	float *d_C1, *d_C2, *d_A1, *d_A2, *d_B1, *d_B2;
	cudaMalloc((void **) &d_C1, memoria); cudaMalloc((void **) &d_C2, memoria);
	cudaMalloc((void **) &d_A1, memoria); cudaMalloc((void **) &d_A2, memoria);
	cudaMalloc((void **) &d_B1, memoria); cudaMalloc((void **) &d_B2, memoria);
	
	/*Inicializar*/
	for(int i=0; i<SIZE*SIZE; i++){
		A1[i] = 1.0; 
		B1[i] = 2.0; 
		C1[i] = 0.0;  
			A2[i] = 1.1; 
			B2[i] = 2.1; 
			C2[i] = 0.0;}

TimerStart( tTotal ); TimerStart( tComm );
	/*Copia a device*/	    
    cudaMemcpy(d_C1, C1, memoria, cudaMemcpyHostToDevice); cudaMemcpy(d_C2, C2, memoria, cudaMemcpyHostToDevice);
    cudaMemcpy(d_A1, A1, memoria, cudaMemcpyHostToDevice); cudaMemcpy(d_A2, A2, memoria, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B1, B1, memoria, cudaMemcpyHostToDevice); cudaMemcpy(d_B2, B2, memoria, cudaMemcpyHostToDevice);
TimerStop(tComm);	
	
TimerStart( tComp );	
	/*Kernel*/
	kernel_recEq<<<grid, threads>>>(d_C1, d_A1, d_B1, SIZE, ITER);
	kernel_recEq<<<grid, threads>>>(d_C2, d_A2, d_B2, SIZE, ITER);
	cudaDeviceSynchronize();
TimerStop(tComp);	
	
TimerContinue(tComm);	
	/*Copia a Host*/
	cudaMemcpy(C1, d_C1, memoria, cudaMemcpyDeviceToHost); cudaMemcpy(C2, d_C2, memoria, cudaMemcpyDeviceToHost);
	cudaDeviceSynchronize();
TimerStop(tComm);
TimerStop( tTotal );

	printf("\nVERSION SINCRONA************\n" );
	printf("Clock Total: %.8lf <--\n", TimerGetTime(tTotal) );
	printf("Clock Comm: %.8lf\n", TimerGetTime(tComm) );
	printf("Clock Comp: %.8lf\n", TimerGetTime(tComp) );
	printf("Result %lf\n",C1[0]);
	
	/*free*/
	free(C1); free(C2);
	free(A1); free(A2);
	free(B1); free(B2);
	cudaFree(d_C1); cudaFree(d_C2);
	cudaFree(d_A1); cudaFree(d_A2);
	cudaFree(d_B1); cudaFree(d_B2);
	
	/*Version asincrona*/												///////////////////////////////////

	/*Declaración y reserva de memoria*/
	float *C1A, *C2A; cudaMallocHost ( &C1A, memoria ) ; cudaMallocHost ( &C2A, memoria ) ; 
	float *A1A, *A2A; cudaMallocHost ( &A1A, memoria ) ; cudaMallocHost ( &A2A, memoria ) ; 
	float *B1A, *B2A; cudaMallocHost ( &B1A, memoria ) ; cudaMallocHost ( &B2A, memoria ) ; 
		
	float *d_C1A, *d_C2A, *d_A1A, *d_A2A, *d_B1A, *d_B2A;
	cudaMalloc((void **) &d_C1A, memoria); cudaMalloc((void **) &d_C2A, memoria);
	cudaMalloc((void **) &d_A1A, memoria); cudaMalloc((void **) &d_A2A, memoria);
	cudaMalloc((void **) &d_B1A, memoria); cudaMalloc((void **) &d_B2A, memoria);

	/*Inicializar*/
	for(int i=0; i<SIZE*SIZE; i++){
		A1A[i] = 1.0; 
		B1A[i] = 2.0; 
		C1A[i] = 0.0;  
			A2A[i] = 1.1; 
			B2A[i] = 2.1; 
			C2A[i] = 0.0;}		

	/*Streams*/
	cudaStream_t stream1, stream2;
	cudaStreamCreate ( &stream1) ; cudaStreamCreate ( &stream2) ; 

TimerStart( tTotalA );	
	/*Copia al device*/
	cudaMemcpyAsync(d_C1A, C1A, memoria, cudaMemcpyHostToDevice, stream1);
	cudaMemcpyAsync(d_A1A, A1A, memoria, cudaMemcpyHostToDevice, stream1);
	cudaMemcpyAsync(d_B1A, B1A, memoria, cudaMemcpyHostToDevice, stream1);	
		
	kernel_recEq<<<grid, threads, 0, stream1>>>(d_C1A, d_A1A, d_B1A, SIZE, ITER);		

	cudaMemcpyAsync(d_C2A, C2A, memoria, cudaMemcpyHostToDevice, stream2);
	cudaMemcpyAsync(d_A2A, A2A, memoria, cudaMemcpyHostToDevice, stream2);
	cudaMemcpyAsync(d_B2A, B2A, memoria, cudaMemcpyHostToDevice, stream2);

	kernel_recEq<<<grid, threads, 0, stream2>>>(d_C2A, d_A2A, d_B2A, SIZE, ITER);		
	
	cudaMemcpyAsync(C1A, d_C1A, memoria, cudaMemcpyDeviceToHost, stream1); 
	cudaMemcpyAsync(C2A, d_C2A, memoria, cudaMemcpyDeviceToHost, stream2);
	cudaDeviceSynchronize();
TimerStop( tTotalA );

	printf("\nVERSION A_SINCRONA************\n" );
	printf("Clock Total A: %.8lf <--\n", TimerGetTime(tTotalA) );
	printf("Result %lf\n",C1A[0]);
	printf("\nPorcentaje de mejora %.3lf\n",  (100.0 - (TimerGetTime(tTotalA)*100.0)/TimerGetTime(tTotal)));

	/*free*/
	cudaFree(C1A); cudaFree(C2A);
	cudaFree(A1A); cudaFree(A2A);
	cudaFree(B1A); cudaFree(B2A);
	cudaFree(d_C1A); cudaFree(d_C2A);
	cudaFree(d_A1A); cudaFree(d_A2A);
	cudaFree(d_B1A); cudaFree(d_B2A);
	cudaStreamDestroy(stream1);
	cudaStreamDestroy(stream2);

	/*Version CPU*/												///////////////////////////////////

	/*Declaración y reserva de memoria*/
	float *CC = (float *) malloc(memoria); 
	float *AC = (float *) malloc(memoria); 
	float *BC = (float *) malloc(memoria); 
	
	/*Inicializar*/
	for(int i=0; i<SIZE*SIZE; i++){
		AC[i] = 1.0; 
		BC[i] = 2.0; 
		CC[i] = 0.0;}
	
TimerStart( tTotalC );	
	float value = 0.000025;	
	for(int i=0; i<ITER; i++)
		for(int j=0; j<SIZE; j++)
			for(int k=0; k<SIZE; k++)
			CC[j*SIZE + k] = CC[j*SIZE + k] + (AC[j*SIZE + k]+ BC[j*SIZE + k])*value;
	
	for(int i=0; i<ITER; i++)
		for(int j=0; j<SIZE; j++)
			for(int k=0; k<SIZE; k++)
			CC[j*SIZE + k] = CC[j*SIZE + k] + (AC[j*SIZE + k]+ BC[j*SIZE + k])*value;	

TimerStop( tTotalC );	
	printf("\nVERSION CPU************\n" );
	printf("Clock Total A: %.8lf <--\n", TimerGetTime(tTotalC) );
	printf("\nPorcentaje de mejora %.3lf\n",  (100.0 - (TimerGetTime(tTotalC)*100.0)/TimerGetTime(tTotalA)));
	
	/*liberacion*/
	free(CC); free(AC); free(BC);

	return 0;
}

