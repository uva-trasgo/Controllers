#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include "timer.h"
#include "CAL.h"

#define IMAD(a, b, c) ( __mul24((a), (b)) + (c) )
#define SEED 6834723
#define cKernelRadius 1 
#define rKernelRadius 1
#define alpha 1.0
int ITER = 1;



hit_tileNewType( float );
hit_ktileNewType( float );

/*Tiempos*/
Timer tTotal, tTotalA, tComp, tComm;

CAL_KERNEL_GPU_CHAR_STATIC(convolutionRowsKernel, 2, full, low, low);
CAL_KERNEL(convolutionRowsKernel, dGPU, KHitTile_float imgDst, KHitTile_float imgSrc, 
									int imageW, int imageH, KHitTile_float rKernel)
{
    const int ix = IMAD(blockDim.x, blockIdx.x, threadIdx.x);
    const int iy = IMAD(blockDim.y, blockIdx.y, threadIdx.y);

    if (ix >= imageW || iy >= imageH)
    {
	return;
    }
	
    float sum = 0;
    
    int d = 0;
    for (int k = -rKernelRadius; k <= rKernelRadius; k++)
    {
	//d = x + (float)k;
	d = ix + k;
	if ((d >= 0) && (d < imageW))
	{
	    sum += (hit_tileElemAtNoStride(imgSrc, 2, d, iy) * hit_tileElemAtNoStride(rKernel, 1, rKernelRadius + k));

	}
    }

   hit_tileElemAtNoStride(imgDst, 2, ix, iy) = sum;

}

CAL_KERNEL_GPU_CHAR_STATIC(convolutionColumnsKernel, 2, full, low, low);
CAL_KERNEL(convolutionColumnsKernel, dGPU,	KHitTile_float imgDst, 
											KHitTile_float imgSrc, 
											int imageW, int imageH, KHitTile_float cKernel)
{
    const   int ix = IMAD(blockDim.x, blockIdx.x, threadIdx.x);
    const   int iy = IMAD(blockDim.y, blockIdx.y, threadIdx.y);


    if (ix >= imageW || iy >= imageH)
    {
        return;
    }

    float sum = 0;
    //float d = 0;
    int d = 0;
    for (int k = -cKernelRadius; k <= cKernelRadius; k++)
    {
        d = iy + k;
        if ((d >= 0) && (d < imageH))
	{

	    sum += (hit_tileElemAtNoStride(imgSrc, 2, ix, d) * hit_tileElemAtNoStride(cKernel, 1, cKernelRadius + k));
	}
    }

    hit_tileElemAtNoStride(imgDst, 2, ix, iy) = sum*sum;

}

CAL_KERNEL_GPU_CHAR_STATIC(elementWiseSqRt, 2, full, low, low);
CAL_KERNEL(elementWiseSqRt, dGPU, KHitTile_float matrix_x, int imageW, int imageH)
{
    const   int ix = IMAD(blockDim.x, blockIdx.x, threadIdx.x);
    const   int iy = IMAD(blockDim.y, blockIdx.y, threadIdx.y);

    if (ix >= imageW || iy >= imageH)
    {
        return;
    }

    hit_tileElemAtNoStride(matrix_x, 2, ix, iy) = sqrt(hit_tileElemAtNoStride(matrix_x, 2, ix, iy));
}//elementWiseSqRt


CAL_KERNEL_GPU_CHAR_STATIC(kernel_saxpy, 2, full, low, low);
CAL_KERNEL(kernel_saxpy, dGPU, KHitTile_float matrix_x, KHitTile_float matrix_y)
{
    const  int col = IMAD(blockDim.x, blockIdx.x, threadIdx.x);
    const  int row = IMAD(blockDim.y, blockIdx.y, threadIdx.y);

	
    hit_tileElemAtNoStride(matrix_y, 2, row, col) =  alpha*hit_tileElemAtNoStride(matrix_x, 2, row, col) +
														hit_tileElemAtNoStride(matrix_y, 2, row, col);
}//saxpy

CAL_KERNEL_PROTO(kernel_saxpy,
				1, dGPU,
				2, IN, HitTile_float, matrix_x, IO, HitTile_float, matrix_y);
CAL_KERNEL_PROTO(convolutionColumnsKernel,
				1, dGPU,
				5, IO, HitTile_float, imgDst, IN, HitTile_float, 
					imgSrc, INVAL, int, imageW, INVAL, int, imageH, IN, HitTile_float, cKernel);
CAL_KERNEL_PROTO(convolutionRowsKernel, 
				1, dGPU,
				5, IO, HitTile_float, imgDst, IN, HitTile_float, imgSrc, 
				INVAL, int, imageW, INVAL, int, imageH, IN, HitTile_float, rKernel);
CAL_KERNEL_PROTO(elementWiseSqRt,
				1, dGPU,
				3, IO, HitTile_float, matrix_x, INVAL, int, imageW, INVAL, int, imageH);





void sobelpl_cudaSYN(int SIZE, int gpu){

	/*Variables*/
	HitTile_float h_KernelGxC, h_KernelGxR, imgOriginal, imgTemp, imgGx, imgGy;
	unsigned int memoria = SIZE*SIZE*sizeof(float);
    hit_tileDomain(&h_KernelGxC, float, 1, 3);
    hit_tileDomain(&h_KernelGxR, float, 1, 3);
    hit_tileDomain(&imgOriginal, float, 2, SIZE, SIZE);
    hit_tileDomain(&imgTemp, float, 2, SIZE, SIZE);
    hit_tileDomain(&imgGx, float, 2, SIZE, SIZE);
    hit_tileDomain(&imgGy, float, 2, SIZE, SIZE);
 
    hit_tileAlloc(&h_KernelGxC);
    hit_tileAlloc(&h_KernelGxR);
    hit_tileAlloc(&imgOriginal);
    hit_tileAlloc(&imgTemp);
    hit_tileAlloc(&imgGx);
    hit_tileAlloc(&imgGy);
    
   // INICIALIZA LAS MATRICES
   srand48( SEED );
    for (unsigned int i = 0; i < SIZE; i++)    
		for (unsigned int j = 0; j < SIZE; j++)
	            hit_tileElemAt(imgOriginal, 2, i, j) = (float)(drand48());
    
    hit_tileElemAt(h_KernelGxC, 1, 0) = 1;
    hit_tileElemAt(h_KernelGxC, 1, 1) = 2;
    hit_tileElemAt(h_KernelGxC, 1, 2) = 1;
    hit_tileElemAt(h_KernelGxR, 1, 0) = -1;
    hit_tileElemAt(h_KernelGxR, 1, 1) = 0;
    hit_tileElemAt(h_KernelGxR, 1, 2) = 1;
    
    CALThread threads;
    CALThreadInit(threads, 2, SIZE, SIZE);
 
 	// Controller
	CALCntrl comm;
	CAL_CntrlCreate(&comm, CAL_CNTRL_GPU, gpu);

TimerStart(tTotal); TimerStart(tComm);TimerStart(tComp); TimerStop(tComp);  
    CAL_CntrlAttach  (&comm, (HitTile*)&h_KernelGxC);
    CAL_CntrlAttach  (&comm, (HitTile*)&h_KernelGxR);
    CAL_CntrlAttach  (&comm, (HitTile*)&imgOriginal);
    CAL_CntrlAttach	 (&comm, (HitTile*)&imgTemp);    
    CAL_CntrlInternal  (&comm, (HitTile*)&imgGx);    
    CAL_CntrlAttach  (&comm, (HitTile*)&imgGy);    
CAL_CntrlSync(comm);
TimerStop(tComm);

for (int iter=0; iter < ITER; iter++){  
TimerContinue(tComp);        
    CAL_CntrlLaunch(comm, convolutionRowsKernel, threads, imgTemp, imgOriginal, SIZE, SIZE, h_KernelGxR);    
	CAL_CntrlLaunch(comm, convolutionColumnsKernel, threads, imgGx, imgTemp, SIZE, SIZE, h_KernelGxC);
	CAL_CntrlLaunch(comm, convolutionRowsKernel, threads, imgTemp, imgOriginal, SIZE, SIZE, h_KernelGxC);
	CAL_CntrlLaunch(comm, convolutionColumnsKernel, threads, imgGy, imgTemp, SIZE, SIZE, h_KernelGxR);
	CAL_CntrlLaunch(comm, kernel_saxpy, threads, imgGx, imgGy); //C = αAB + βC
	CAL_CntrlLaunch(comm, elementWiseSqRt, threads, imgGy, SIZE, SIZE);
CAL_CntrlSync(comm);
TimerStop(tComp);
TimerContinue(tComm);

	CAL_CntrlMoveFrom(&comm, (HitTile*)&imgGy); 
	//CAL_CntrlDetach(&comm, (HitTile*)&imgGy);
	CAL_CntrlSync(comm);
TimerStop(tComm);
}//for iter
TimerStop(tTotal);
	CAL_CntrlDestroy(&comm); 

 
	printf("\nVERSION SINCRONA************\n" );
	printf("Tamaño %d %d\n", SIZE, SIZE);
	printf("Clock Total: %.8lf <--\n", TimerGetTime(tTotal) );
	printf("Clock Comm: %.8lf\n", TimerGetTime(tComm) );
	printf("Clock Comp: %.8lf\n", TimerGetTime(tComp) );	
	printf("Iteraciones: %d\n", ITER);  

    printf("Resultado: %lf\n", hit_tileElemAtNoStride(imgGy, 1, 0));
    
    hit_tileFree(h_KernelGxC); 
    hit_tileFree(h_KernelGxR); 
    hit_tileFree(imgOriginal); 
    hit_tileFree(imgTemp); 
    hit_tileFree(imgGx); 
    hit_tileFree(imgGy);

}//sobelpl_cudaSYN

void sobelpl_cudaASYN(int SIZE, int gpu){

	/*Variables*/
	HitTile_float h_KernelGxC, h_KernelGxR, imgOriginal, imgTemp, imgGx, imgGy;
	unsigned int memoria = SIZE*SIZE*sizeof(float);
    hit_tileDomain(&h_KernelGxC, float, 1, 3);
    hit_tileDomain(&h_KernelGxR, float, 1, 3);
    hit_tileDomain(&imgOriginal, float, 2, SIZE, SIZE);
    hit_tileDomain(&imgTemp, float, 2, SIZE, SIZE);
    hit_tileDomain(&imgGx, float, 2, SIZE, SIZE);
    hit_tileDomain(&imgGy, float, 2, SIZE, SIZE);
 
    hit_tileAlloc(&h_KernelGxC);
    hit_tileAlloc(&h_KernelGxR);
    hit_tileAlloc(&imgOriginal);
    hit_tileAlloc(&imgTemp);
    hit_tileAlloc(&imgGx);
    hit_tileAlloc(&imgGy);
    
   // INICIALIZA LAS MATRICES
   srand48( SEED );
    for (unsigned int i = 0; i < SIZE; i++)    
		for (unsigned int j = 0; j < SIZE; j++)
	            hit_tileElemAt(imgOriginal, 2, i, j) = (float)(drand48());
    
    hit_tileElemAt(h_KernelGxC, 1, 0) = 1;
    hit_tileElemAt(h_KernelGxC, 1, 1) = 2;
    hit_tileElemAt(h_KernelGxC, 1, 2) = 1;
    hit_tileElemAt(h_KernelGxR, 1, 0) = -1;
    hit_tileElemAt(h_KernelGxR, 1, 1) = 0;
    hit_tileElemAt(h_KernelGxR, 1, 2) = 1;
    
    CALThread threads;
    CALThreadInit(threads, 2, SIZE, SIZE);
 
 	// Controller
	CALCntrl comm;
	CAL_CntrlCreate(&comm, CAL_CNTRL_GPU, gpu);

	//Streams
	cudaStream_t stream1, stream2, stream3, stream4, stream5, stream6;
	cudaStreamCreate ( &stream1) ; cudaStreamCreate ( &stream2) ; cudaStreamCreate ( &stream3) ;
	cudaStreamCreate ( &stream4) ; cudaStreamCreate ( &stream5) ; cudaStreamCreate ( &stream6) ;
	
	//Attach
    CAL_CntrlAttachAsyn  (&comm, (HitTile*)&h_KernelGxC, stream1);
    CAL_CntrlAttachAsyn  (&comm, (HitTile*)&h_KernelGxR, stream2);
    CAL_CntrlAttachAsyn  (&comm, (HitTile*)&imgOriginal, stream3);
    CAL_CntrlAttachAsyn	 (&comm, (HitTile*)&imgTemp, stream4);    
    CAL_CntrlAttachAsyn  (&comm, (HitTile*)&imgGx, stream5);    
    CAL_CntrlAttachAsyn  (&comm, (HitTile*)&imgGy, stream6);  
CAL_CntrlSync(comm);

TimerStart(tTotalA);         
	CAL_CntrlMoveToAsyn  (&comm, (HitTile*)&h_KernelGxC, stream1);
    CAL_CntrlMoveToAsyn  (&comm, (HitTile*)&h_KernelGxR, stream2);
    CAL_CntrlMoveToAsyn  (&comm, (HitTile*)&imgOriginal, stream3);
    CAL_CntrlMoveToAsyn	 (&comm, (HitTile*)&imgTemp, stream4);    
    CAL_CntrlMoveToAsyn  (&comm, (HitTile*)&imgGx, stream5);    
    CAL_CntrlMoveToAsyn  (&comm, (HitTile*)&imgGy, stream6);          
CAL_CntrlSync(comm);        



for (int iter=0; iter < ITER; iter++){  

    CAL_CntrlLaunchCKernel(comm, convolutionRowsKernel, threads, stream1, imgTemp, imgOriginal, SIZE, SIZE, h_KernelGxR);    
	CAL_CntrlLaunchCKernel(comm, convolutionColumnsKernel, threads, stream1, imgGx, imgTemp, SIZE, SIZE, h_KernelGxC);
	CAL_CntrlLaunchCKernel(comm, convolutionRowsKernel, threads, stream1, imgTemp, imgOriginal, SIZE, SIZE, h_KernelGxC);
	CAL_CntrlLaunchCKernel(comm, convolutionColumnsKernel, threads, stream1, imgGy, imgTemp, SIZE, SIZE, h_KernelGxR);
	CAL_CntrlLaunchCKernel(comm, kernel_saxpy, threads, stream1, imgGx, imgGy); //C = αAB + βC
	CAL_CntrlLaunchCKernel(comm, elementWiseSqRt, threads, stream1, imgGy, SIZE, SIZE);


	CAL_CntrlMoveFromAsyn(&comm, (HitTile*)&imgGy, stream2); 	
	CAL_CntrlSync(comm);

}//for iter
CAL_CntrlSync(comm);
TimerStop(tTotalA);
	CAL_CntrlDestroy(&comm); 

 
	printf("\nVERSION A_SINCRONA************\n" );
	printf("Tamaño %d %d\n", SIZE, SIZE);
	printf("Clock Total: %.8lf <--\n", TimerGetTime(tTotalA) );
	printf("Iteraciones: %d\n", ITER);  
    printf("Resultado: %lf\n", hit_tileElemAtNoStride(imgGy, 1, 0));
    printf("\nPorcentaje de mejora %.3lf\n",  (100.0 - (TimerGetTime(tTotalA)*100.0)/TimerGetTime(tTotal)));
    hit_tileFree(h_KernelGxC); 
    hit_tileFree(h_KernelGxR); 
    hit_tileFree(imgOriginal); 
    hit_tileFree(imgTemp); 
    hit_tileFree(imgGx); 
    hit_tileFree(imgGy);
    cudaStreamDestroy(stream1);
	cudaStreamDestroy(stream2);
	cudaStreamDestroy(stream3);
	cudaStreamDestroy(stream4);
	cudaStreamDestroy(stream5);
	cudaStreamDestroy(stream6);

}//sobelpl_cudaASYN


/*
 * Main program to perform matrix multiplication
 */
int main(int argc, char *argv[]) {

    omp_set_nested(1);
    omp_set_num_threads(3);
    #pragma omp parallel 
    {
    #pragma omp single 
     {


	if ( argc!=5 ) {
		fprintf(stderr, "Usage: %s <matrixSize> <matrixSize> <iter> <GPU>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	/*Entradas*/
	int SIZE	= atoi( argv[1] );
	ITER	= atoi( argv[3] );
	int GPU		= atoi( argv[4] );	
	
	/*timers*/
    TimerCreate( tTotal ); TimerCreate( tTotalA );
    TimerCreate( tComp );
    TimerCreate( tComm );   	
	
	sobelpl_cudaSYN(SIZE, GPU);
	sobelpl_cudaASYN(SIZE, GPU);
	
	
	
	/*Frees*/
	TimerDestroy( tTotal );
	TimerDestroy( tTotalA );
	TimerDestroy( tComp );
	TimerDestroy( tComm );	
	
	
	
	CAL_CntrlFinish();
	
	return 0;
}






















