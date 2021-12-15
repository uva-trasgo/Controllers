#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include "timer.h"
#include "CAL.h"

/*Relojes*/
HitClock	mainClock;


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
	//printf("kernel A1 %lf   B1 %lf  C1 %lf \n",hit_tileElemAt(A, 1, 0), hit_tileElemAt(B, 1, 0), hit_tileElemAt(C, 1, 0));
	//printf("kernel  %lf  %lf  %lf \n", hit_tileElemAtNoStride(C, 2, fila, columna), hit_tileElemAtNoStride(A, 2, fila, columna), hit_tileElemAtNoStride(B, 2, fila, columna));															
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

		hit_comInit( &argc, &argv );

        // 1. Init Controllers library									
        omp_set_nested(1);
        omp_set_num_threads(3);
        #pragma omp parallel 
        {
        #pragma omp single 
         {


	if ( argc!=5 ) {
		fprintf(stderr, "Usage: %s <matrixSize> <matrixSize> <iter> <GPU> \n\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	
	/*Entradas*/
	int SIZE	= atoi( argv[1] );
	int ITER	= atoi( argv[3] );
	int GPU		= atoi( argv[4] );

	//Número de procesos
	int size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);	
	GPU = rank;
	
	//Caracteristicas GPU
	int nDevices;
	cudaGetDeviceCount(&nDevices);
	if (rank == 0) printf("rank %d;   size %d;  devices %d\n", rank, size, nDevices);

/*  for (int i = 0; i < nDevices; i++) {
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, i);
    printf("Device Number: %d\n", i);
    printf("  Device name: %s\n", prop.name);
    printf("  Memory Clock Rate (KHz): %d\n",
           prop.memoryClockRate);
    printf("  Memory Bus Width (bits): %d\n",
           prop.memoryBusWidth);
    printf("  Peak Memory Bandwidth (GB/s): %f\n\n",
           2.0*prop.memoryClockRate*(prop.memoryBusWidth/8)/1.0e6);
  }*/


    /* 1. CREATE VIRTUAL TOPOLOGY */
    HitTopology topo = hit_topology( plug_topArray2DComplete );

	/* 2. DECLARE FULL MATRIX WITHOUT MEMORY */
	HitTile_float C0;
	HitTile_float A0;
	HitTile_float B0;
	hit_tileDomain( &C0, float, 2, SIZE, SIZE );
	hit_tileDomain( &A0, float, 2, SIZE, SIZE );
	hit_tileDomain( &B0, float, 2, SIZE, SIZE );
	
	/* 3. COMPUTE PARTITION */
	//HitShape parallelShape = hit_tileShape( C0 );
	HitLayout matLayoutC = hit_layout( plug_layBlocks, topo, hit_tileShape( C0 ) );
	HitLayout matLayoutA = hit_layout( plug_layBlocks, topo, hit_tileShape( A0 ) );
	HitLayout matLayoutB = hit_layout( plug_layBlocks, topo, hit_tileShape( B0 ) );

	/* 4. ACTIVE PROCESSES */

	/*Declaración*/
	HitTile_float C1; 
	HitTile_float A1; 
	HitTile_float B1;
	
	hit_tileSelect( &C1, &C0, hit_layShape(matLayoutC) );
	hit_tileSelect( &A1, &A0, hit_layShape(matLayoutA) );
	hit_tileSelect( &B1, &B0, hit_layShape(matLayoutB) );
	hit_tileAlloc( &C1 );
	hit_tileAlloc( &A1 );
	hit_tileAlloc( &B1 );
	 
	//hit_tileDomainAlloc( &C1, float, 2, SIZE, SIZE ); 
	//hit_tileDomainAlloc( &A1, float, 2, SIZE, SIZE ); 
	//hit_tileDomainAlloc( &B1, float, 2, SIZE, SIZE ); 
	
	float uno	= 1.0; 
	float dos	= 2.0; 
	float zero	= 0.0; 
	hit_tileFill( &A1, &uno );
	hit_tileFill( &B1, &dos );
	hit_tileFill( &C1, &zero );
	
	/*Inicizalizar*/
//	for(int i=0; i<SIZE*SIZE; i++)
//		{hit_tileElemAt(A1, 1, i) = 1.0; 
//			hit_tileElemAt(B1, 1, i) = 2.0; 
//			hit_tileElemAt(C1, 1, i) =  0.0;}
	

	/*hilos*/	
	CALThread threads;
	//CALThreadInit( threads, 2, SIZE, SIZE );
	CALThreadInit( threads, 2, hit_tileDimCard(C1,0), hit_tileDimCard(C1,1) );
	printf("Cardinalidades %d %d rank %d\n thread %d %d\n", hit_tileDimCard(C1,0), hit_tileDimCard(C1,1), rank, 
	threads.x, threads.y);

	/*controlador*/
	CALCntrl comm;
	CAL_CntrlCreate(&comm, CAL_CNTRL_GPU, GPU);	

	/* 0. INIT CLOCKS */
	hit_clockSynchronizeAll();
	hit_clockStart( mainClock );

	/*Copia a device*/	  
	CAL_CntrlAttach(&comm, (HitTile*)&C1); 
	CAL_CntrlAttach(&comm, (HitTile*)&A1); 
	CAL_CntrlAttach(&comm, (HitTile*)&B1); 
	
	/*kernel*/
	CAL_CntrlLaunch(comm, kernel_recEq, threads, C1, A1, B1);		
	
	/*Copia a host*/	  
	CAL_CntrlDetach(&comm, (HitTile*)&C1); 
	CAL_CntrlSync(comm);	
	//printf("Point A1 %lf   B1 %lf  C1 %lf \n",hit_tileElemAt(A1, 1, 10), hit_tileElemAt(B1, 1, 10), hit_tileElemAt(C1, 1, 10));	

	hit_clockStop( mainClock );
	hit_clockReduce( matLayoutC, mainClock );

	/*Destrucción controller*/
	CAL_CntrlDestroy(&comm);

	hit_clockPrintMax( mainClock );
	printf("Resultado %lf rank %d\n", hit_tileElemAt(C1, 1, 0), rank);
	
	/*Free*/
	hit_tileFree(C1);
	hit_tileFree(A1);
	hit_tileFree(B1);
	hit_layFree( matLayoutC );
	hit_layFree( matLayoutA );
	hit_layFree( matLayoutB );
	hit_topFree( topo );

	CAL_CntrlFinish();
	return 0;
}

