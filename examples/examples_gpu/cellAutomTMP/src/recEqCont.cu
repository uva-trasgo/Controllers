#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include "timer.h"
#include "CAL.h"


/*Tipo de datos*/
hit_tileNewType( float );
hit_ktileNewType( float );

/* A. GPU Kernel charazterization */
CAL_KERNEL_GPU_CHAR_STATIC(kernel_recEq, 2, fixed, square, 32);

/* B. CPU Kernel implementation */
CAL_KERNEL(kernel_recEq, dGPU, KHitTile_float C, KHitTile_float A, KHitTile_float B){


	int fila = blockIdx.y*blockDim.y +threadIdx.y;
	int columna = blockIdx.x*blockDim.x +threadIdx.x;
	float value = 0.000025;
	int iter = 250;	
	//if(fila <= C.origAcumCard[1] - 1 && columna <= C.origAcumCard[1] -1){	
		for(int i=0; i<iter; i++)
			hit_tileElemAtNoStride(C, 2, fila, columna) = hit_tileElemAtNoStride(C, 2, fila, columna) + 
															(hit_tileElemAtNoStride(A, 2, fila, columna)+ 
																hit_tileElemAtNoStride(B, 2, fila, columna))*value;
	//}
}//kernel_recEq

/* C. Defining kernel prototypes */
CAL_KERNEL_PROTO( kernel_recEq,                            	// Name
                  1, dGPU,									// Implementations
                  3, OUT, HitTile_float, C, IN, HitTile_float, A, IN, HitTile_float, B
                );

/*
 * Main program to perform matrix multiplication
 */
int main(int argc, char *argv[]) {

        // 1. Init Controllers library									
        omp_set_nested(1);
        omp_set_num_threads(3);
        #pragma omp parallel 
        {
        #pragma omp single 
         {


	if ( argc!=6 ) {
		fprintf(stderr, "Usage: %s <matrixSize> <matrixSize> <iter> <GPU> <Version>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	
    Timer tTotal, tTotalA, tComp, tComm;
    TimerCreate( tTotal ); 
    TimerCreate( tTotalA );
    TimerCreate( tComp );
    TimerCreate( tComm );

	/*Entradas*/
	int SIZE	= atoi( argv[1] );
	int ITER	= atoi( argv[3] );
	int GPU		= atoi( argv[4] );
	int VERSION	= atoi( argv[5] );
	unsigned int memoria = SIZE*SIZE*sizeof(float);
	//printf("Tamaño %d x %d,  Iteraciones %d,  GPU %d,  Version %d \n", SIZE, SIZE, ITER, GPU, VERSION);		

	/*Declaración*/
	HitTile_float C1; HitTile_float C2;
	HitTile_float A1; HitTile_float A2;
	HitTile_float B1; HitTile_float B2;
	hit_tileDomainAlloc( &C1, float, 2, SIZE, SIZE ); hit_tileDomainAlloc( &C2, float, 2, SIZE, SIZE ); 
	hit_tileDomainAlloc( &A1, float, 2, SIZE, SIZE ); hit_tileDomainAlloc( &A2, float, 2, SIZE, SIZE ); 
	hit_tileDomainAlloc( &B1, float, 2, SIZE, SIZE ); hit_tileDomainAlloc( &B2, float, 2, SIZE, SIZE ); 
	
	/*Inicizalizar*/
	for(int i=0; i<SIZE*SIZE; i++)
		{hit_tileElemAt(A1, 1, i) = 1.0; 
			hit_tileElemAt(B1, 1, i) = 2.0; 
			hit_tileElemAt(C1, 1, i) =  0.0;  
			hit_tileElemAt(A2, 1, i) = 1.1; 
			hit_tileElemAt(B2, 1, i) = 2.1; 
			hit_tileElemAt(C2, 1, i) =  0.0;}
	
	/*controlador*/
	CALCntrl comm;
	CAL_CntrlCreate(&comm, CAL_CNTRL_GPU, GPU);
	
	/*hilos*/	
	CALThread threads;
	CALThreadInit( threads, 2, SIZE, SIZE );

TimerStart( tTotal ); TimerStart( tComm );		
	/*Copia a device*/	  
	CAL_CntrlAttach(&comm, (HitTile*)&C1); CAL_CntrlAttach(&comm, (HitTile*)&C2);
	CAL_CntrlAttach(&comm, (HitTile*)&A1); CAL_CntrlAttach(&comm, (HitTile*)&A2);
	CAL_CntrlAttach(&comm, (HitTile*)&B1); CAL_CntrlAttach(&comm, (HitTile*)&B2);
CAL_CntrlSync(comm);
TimerStop( tComm ); 		

TimerStart( tComp );
	/*kernel*/
	CAL_CntrlLaunch(comm, kernel_recEq, threads, C1, A1, B1);	
	CAL_CntrlLaunch(comm, kernel_recEq, threads, C2, A2, B2);	
CAL_CntrlSync(comm);	
TimerStop( tComp );	

TimerContinue( tComm );	
	/*Copia a host*/	  
	CAL_CntrlDetach(&comm, (HitTile*)&C1); CAL_CntrlDetach(&comm, (HitTile*)&C2);
CAL_CntrlSync(comm);		
TimerStop( tTotal ); TimerStop( tComm );

	/*Destrucción controller*/
	CAL_CntrlDestroy(&comm);

	printf("\nVERSION SINCRONA************\n" );
	printf("Clock Total: %.8lf <--\n", TimerGetTime(tTotal) );
	printf("Clock Comm: %.8lf\n", TimerGetTime(tComm) );
	printf("Clock Comp: %.8lf\n", TimerGetTime(tComp) );
	printf("Resultado %lf\n", hit_tileElemAt(C1, 1, 0));
	
	/*Free*/
	hit_tileFree(C1); hit_tileFree(C2);
	hit_tileFree(A1); hit_tileFree(A2);
	hit_tileFree(B1); hit_tileFree(B2);
	
	/*Version Asincrona*/												//////////////////////////////

	/*Declaración y creación de memoria*/
	HitTile_float C1A; HitTile_float C2A;
	HitTile_float A1A; HitTile_float A2A;
	HitTile_float B1A; HitTile_float B2A;
	hit_tileDomainAlloc( &C1A, float, 2, SIZE, SIZE ); hit_tileDomainAlloc( &C2A, float, 2, SIZE, SIZE ); 
	hit_tileDomainAlloc( &A1A, float, 2, SIZE, SIZE ); hit_tileDomainAlloc( &A2A, float, 2, SIZE, SIZE ); 
	hit_tileDomainAlloc( &B1A, float, 2, SIZE, SIZE ); hit_tileDomainAlloc( &B2A, float, 2, SIZE, SIZE ); 

	/*Inicizalizar*/
	for(int i=0; i<SIZE*SIZE; i++){
					hit_tileElemAt(A1A, 1, i) = 1.0; 
					hit_tileElemAt(B1A, 1, i) = 2.0; 
					hit_tileElemAt(C1A, 1, i) = 0.0;  
					hit_tileElemAt(A2A, 1, i) = 1.1; 
					hit_tileElemAt(B2A, 1, i) = 2.1; 
					hit_tileElemAt(C2A, 1, i) =  0.0;}

	/*controlador*/
	CALCntrl commA;
	CAL_CntrlCreate(&commA, CAL_CNTRL_GPU, GPU);

	/*Streams*/
	cudaStream_t stream1, stream2;
	cudaStreamCreate ( &stream1) ; cudaStreamCreate ( &stream2) ; 	

	/*Copia al device*/
	CAL_CntrlAttachAsyn(&commA, (HitTile*)&C1A, stream1); 
	CAL_CntrlAttachAsyn(&commA, (HitTile*)&A1A, stream1); 
	CAL_CntrlAttachAsyn(&commA, (HitTile*)&B1A, stream1); 	
	CAL_CntrlAttachAsyn(&commA, (HitTile*)&C2A, stream2); 
	CAL_CntrlAttachAsyn(&commA, (HitTile*)&A2A, stream2); 
	CAL_CntrlAttachAsyn(&commA, (HitTile*)&B2A, stream2); 
	CAL_CntrlSync(commA);
	
TimerStart( tTotalA );	
	/*Copia a device*/	  
	CAL_CntrlMoveToAsyn(&commA, (HitTile*)&C1A, stream1); 
	CAL_CntrlMoveToAsyn(&commA, (HitTile*)&A1A, stream1); 
	CAL_CntrlMoveToAsyn(&commA, (HitTile*)&B1A, stream1); 
	
	/*kernel*/
	CAL_CntrlLaunchCKernel(commA, kernel_recEq, threads, stream1, C1A, A1A, B1A);	

	/*Copia a device*/	  
	CAL_CntrlMoveToAsyn(&commA, (HitTile*)&C2A, stream2); 
	CAL_CntrlMoveToAsyn(&commA, (HitTile*)&A2A, stream2); 
	CAL_CntrlMoveToAsyn(&commA, (HitTile*)&B2A, stream2); 
	
	/*kernel*/
	CAL_CntrlLaunchCKernel(commA, kernel_recEq, threads, stream2, C2A, A2A, B2A);			

	/*Copia a host*/	  
	CAL_CntrlDetachAsyn(&commA, (HitTile*)&C1A, stream1); CAL_CntrlDetachAsyn(&commA, (HitTile*)&C2A, stream2);
	CAL_CntrlSync(commA);
TimerStop( tTotalA );
	
	printf("\nVERSION A_SINCRONA************\n" );
	printf("Clock Total: %.8lf <--\n", TimerGetTime(tTotalA) );		
	printf("Resultado %lf\n", hit_tileElemAt(C1A, 1, 0));
	printf("\nPorcentaje de mejora %.3lf\n",  (100.0 - (TimerGetTime(tTotalA)*100.0)/TimerGetTime(tTotal)));
	
	/*Destrucción de controller*/
	CAL_CntrlDestroy(&commA);

	/*Free*/
	hit_tileFree(C1A); hit_tileFree(C2A);
	hit_tileFree(A1A); hit_tileFree(A2A);
	hit_tileFree(B1A); hit_tileFree(B2A);	
	cudaStreamDestroy ( stream1 );	
	cudaStreamDestroy ( stream2 );	
	
	CAL_CntrlFinish();
	return 0;
}

