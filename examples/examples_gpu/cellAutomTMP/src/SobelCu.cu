#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include "timer.h"
#include "CAL.h"

#define IMAD(a, b, c) ( __mul24((a), (b)) + (c) )
#define SEED 6834723
int ITER = 1;

/*Tiempos*/
Timer tTotal, tTotalA, tComp, tComm;

void init_array(float* imgOriginal, float* hKernelGxC, float* hKernelGxR, int SIZE){
	srand48( SEED );
    for (unsigned int i = 0; i < SIZE; i++)
		for (unsigned int j = 0; j < SIZE; j++)
	         imgOriginal[j+(i*SIZE)] = (float)(drand48());

	
    //hKernelGxC = { 1, 2, 1};
    hKernelGxC[0] = 1; hKernelGxC[1] = 2; hKernelGxC[2] = 1;
    //hKernelGxR = {-1, 0, 1};
    hKernelGxR[0] = -1; hKernelGxR[1] = 0; hKernelGxR[2] = 1;
}//init_array

/*
 * 
 * Kernels
 * 
 */

__global__ void convolutionRowsKernel(float* imgDst, float* imgSrc, int imageW, int imageH, float* rKernel, int rKernelRadius)
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
	    sum += (imgSrc[IMAD(d, imageW, iy)] * rKernel[rKernelRadius + k]);
	 
	}
    }
    
   imgDst[IMAD(ix, imageW, iy)] = sum;

}//convolutionRowsKernel


__global__ void convolutionColumnsKernel(float* imgDst, float* imgSrc, int imageW, int imageH, float* cKernel, int cKernelRadius)
{
    const   int ix = IMAD(blockDim.x, blockIdx.x, threadIdx.x);
    const   int iy = IMAD(blockDim.y, blockIdx.y, threadIdx.y);

    if (ix >= imageW || iy >= imageH)
    {
        return;
    }

    float sum = 0;
    int d = 0;
    for (int k = -cKernelRadius; k <= cKernelRadius; k++)
    {
        d = iy + k;
        if ((d >= 0) && (d < imageH))
	{
	    sum += (imgSrc[IMAD(ix, imageH, d)] * cKernel[cKernelRadius + k]);
	}
    }

    imgDst[IMAD(ix, imageW, iy)] = sum*sum;

}//convolutionColumnsKernel

__global__ void saxpy (float* x, float* y, float alpha, int size)
{
	const int col = IMAD(blockDim.x, blockIdx.x, threadIdx.x);
    const int row = IMAD(blockDim.y, blockIdx.y, threadIdx.y);

	y[row*size+col] = alpha*x[row*size+col] + y[row*size+col];
	
}//saxpy


__global__ void elementWiseSqRt (float* matrix_x, int imageW, int imageH)
{
    const   int ix = IMAD(blockDim.x, blockIdx.x, threadIdx.x);
    const   int iy = IMAD(blockDim.y, blockIdx.y, threadIdx.y);

    if (ix >= imageW || iy >= imageH)
    {
        return;
    }

    matrix_x[IMAD(ix, imageW, iy)] = sqrt(matrix_x[IMAD(ix, imageW, iy)]);
}//elementWiseSqRt


void sobelpl_cudaSYN(int SIZE, int gpu){
    
    unsigned int memoria = SIZE*SIZE*sizeof(float);
    float hKernelGxC[3];
    float hKernelGxR[3];
	float *imgOriginal = (float *) malloc(memoria);
	float *hImgGy = (float *) malloc(memoria);
    init_array(imgOriginal, hKernelGxC, hKernelGxR, SIZE);
    
    /*variables*/
    float cu_alpha = 1.0; 
    float cu_beta = 1.0;
    float cu_one  = 1.0;
    float cu_zero = 0.0;
    float *dKernelGxC, *dKernelGxR;
    float *dImgOriginal;
    float *dImgTemp;
    float *dImgGx, *dImgGy;

	cudaSetDevice(gpu); 
	

    cudaMalloc((void**)&dKernelGxC,  sizeof(float)* 3);
    cudaMalloc((void**)&dKernelGxR,  sizeof(float)* 3);
    cudaMalloc((void**)&dImgOriginal,  sizeof(float)*SIZE*SIZE);
    cudaMalloc((void**)&dImgTemp,  sizeof(float)*SIZE*SIZE); //Internal
    cudaMalloc((void**)&dImgGx,  sizeof(float)*SIZE*SIZE); //Internal
    cudaMalloc((void**)&dImgGy,  sizeof(float)*SIZE*SIZE); //Output only




	cudaMemcpy(dImgOriginal, imgOriginal, sizeof(float)*SIZE*SIZE, cudaMemcpyHostToDevice);
TimerStart( tTotal );  TimerStart( tComm ); 
	cudaMemcpy(dKernelGxC, hKernelGxC, sizeof(float)*3, cudaMemcpyHostToDevice);
	cudaMemcpy(dKernelGxR, hKernelGxR, sizeof(float)*3, cudaMemcpyHostToDevice);
 TimerStop( tComm );
	
	
	dim3 threads (128,2);	
	dim3 grid (   (SIZE + threads.x - 1) / threads.x , (SIZE + threads.y - 1) / threads.y );
TimerStart( tComp );TimerStop( tComp );
for (int iter=0; iter < ITER; iter++){    
TimerContinue( tComp );	
	convolutionRowsKernel<<<grid, threads>>>(dImgTemp, dImgOriginal, SIZE, SIZE, dKernelGxR, 1);    
    convolutionColumnsKernel<<<grid, threads>>>(dImgGx, dImgTemp, SIZE, SIZE, dKernelGxC, 1);
    convolutionRowsKernel<<<grid, threads>>>(dImgTemp, dImgOriginal, SIZE, SIZE, dKernelGxC, 1);
    convolutionColumnsKernel<<<grid, threads>>>(dImgGy, dImgTemp, SIZE, SIZE, dKernelGxR, 1);
	saxpy<<<grid, threads>>> (dImgGx, dImgGy, cu_one, SIZE);
	elementWiseSqRt<<<grid, threads>>>(dImgGy, SIZE, SIZE);
	cudaDeviceSynchronize();
TimerStop( tComp );    	
TimerContinue( tComm );
	cudaMemcpy(hImgGy, dImgGy, sizeof(float)*SIZE*SIZE, cudaMemcpyDeviceToHost);
	//cudaMemcpy(dImgOriginal, dImgGy, sizeof(float)*SIZE*SIZE, cudaMemcpyDeviceToDevice);
TimerStop( tComm );

}//for ITER   
TimerStop( tTotal );	
	printf("\nVERSION SINCRONA************\n" );
	printf("Tamaño %d %d\n", SIZE, SIZE);
	printf("Clock Total: %.8lf <--\n", TimerGetTime(tTotal) );
	printf("Clock Comm: %.8lf\n", TimerGetTime(tComm) );
	printf("Clock Comp: %.8lf\n", TimerGetTime(tComp) );	
	printf("Iteraciones: %d\n", ITER);  
    printf("Resultado: %lf\n", hImgGy[0]);
    
    
	free(imgOriginal);
	free(hImgGy);
    cudaFree(dKernelGxC);cudaFree(dKernelGxR);
    cudaFree(dImgOriginal);
    cudaFree(dImgTemp);
    cudaFree(dImgGx); cudaFree(dImgGy);

}//sobelpl_cuda

void sobelpl_cudaASYN(int SIZE, int gpu){
    
    unsigned int memoria = SIZE*SIZE*sizeof(float);
    float hKernelGxC[3];
    float hKernelGxR[3];
	float *imgOriginal; cudaMallocHost ( &imgOriginal, memoria ) ;
	float *hImgGy; cudaMallocHost ( &hImgGy, memoria ) ;
    init_array(imgOriginal, hKernelGxC, hKernelGxR, SIZE);
    
    
    
    /*variables*/
    float cu_alpha = 1.0; 
    float cu_beta = 1.0;
    float cu_one  = 1.0;
    float cu_zero = 0.0;
    float *dKernelGxC, *dKernelGxR;
    float *dImgOriginal;
    float *dImgTemp;
    float *dImgGx, *dImgGy;

	cudaSetDevice(gpu); 
	
	/*Streams*/
	cudaStream_t stream1, stream2, stream3;
	cudaStreamCreate ( &stream1) ; cudaStreamCreate ( &stream2) ; cudaStreamCreate ( &stream3) ; 
	
    cudaMalloc((void**)&dKernelGxC,  sizeof(float)* 3);
    cudaMalloc((void**)&dKernelGxR,  sizeof(float)* 3);
    cudaMalloc((void**)&dImgOriginal,  sizeof(float)*SIZE*SIZE);
    cudaMalloc((void**)&dImgTemp,  sizeof(float)*SIZE*SIZE); //Internal
    cudaMalloc((void**)&dImgGx,  sizeof(float)*SIZE*SIZE); //Internal
    cudaMalloc((void**)&dImgGy,  sizeof(float)*SIZE*SIZE); //Output only
TimerStart( tTotalA );
	dim3 threads (128,2);	
	dim3 grid (   (SIZE + threads.x - 1) / threads.x , (SIZE + threads.y - 1) / threads.y );


	
	cudaMemcpyAsync(dKernelGxR, hKernelGxR, sizeof(float)*3, cudaMemcpyHostToDevice, stream1);
	cudaMemcpyAsync(dImgOriginal, imgOriginal, memoria, cudaMemcpyHostToDevice, stream1);		
	cudaMemcpyAsync(dKernelGxC, hKernelGxC, sizeof(float)*3, cudaMemcpyHostToDevice, stream2);    

for (int iter=0; iter < ITER; iter++){
	   
	convolutionRowsKernel<<<grid, threads, 0, stream1>>>(dImgTemp, dImgOriginal, SIZE, SIZE, dKernelGxR, 1);
    convolutionColumnsKernel<<<grid, threads, 0, stream1>>>(dImgGx, dImgTemp, SIZE, SIZE, dKernelGxC, 1);
    convolutionRowsKernel<<<grid, threads, 0, stream1>>>(dImgTemp, dImgOriginal, SIZE, SIZE, dKernelGxC, 1);
    convolutionColumnsKernel<<<grid, threads, 0, stream1>>>(dImgGy, dImgTemp, SIZE, SIZE, dKernelGxR, 1);
	saxpy<<<grid, threads, 0, stream1>>> (dImgGx, dImgGy, cu_one, SIZE);
	elementWiseSqRt<<<grid, threads, 0, stream1>>>(dImgGy, SIZE, SIZE);
	
	cudaMemcpyAsync(hImgGy, dImgGy, memoria, cudaMemcpyDeviceToHost, stream2);
	//cudaMemcpy(dImgOriginal, dImgGy, sizeof(float)*SIZE*SIZE, cudaMemcpyDeviceToDevice);
	cudaDeviceSynchronize();
}//for ITER
cudaDeviceSynchronize();   
TimerStop( tTotalA );
	printf("\nVERSION A_SINCRONA************\n" );
	printf("Tamaño %d %d\n", SIZE, SIZE);
	printf("Clock Total: %.8lf <--\n", TimerGetTime(tTotalA) );
	printf("Iteraciones: %d\n", ITER);  
    printf("Resultado: %lf\n", hImgGy[0]);
    printf("\nPorcentaje de mejora %.3lf\n",  (100.0 - (TimerGetTime(tTotalA)*100.0)/TimerGetTime(tTotal)));
    
	cudaFree(imgOriginal);
	cudaFree(hImgGy);
    cudaFree(dKernelGxC);cudaFree(dKernelGxR);
    cudaFree(dImgOriginal);
    cudaFree(dImgTemp);
    cudaFree(dImgGx); cudaFree(dImgGy);
	cudaStreamDestroy(stream1);
	cudaStreamDestroy(stream2);
	cudaStreamDestroy(stream3);
}//sobelpl_cuda





/*
 * Main program to perform matrix multiplication
 */
int main(int argc, char *argv[]) {

	if ( argc!=5 ) {
		fprintf(stderr, "Usage: %s <matrixSize> <matrixSize> <iter> <GPU>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	

    TimerCreate( tTotal ); TimerCreate( tTotalA );
    TimerCreate( tComp );
    TimerCreate( tComm );   

	/*Entradas*/
	int SIZE	= atoi( argv[1] );
	ITER	= atoi( argv[3] );
	int GPU		= atoi( argv[4] );	
	
	/*variables etc..*/

/*	
	float hKernelGxCA[3];
    float hKernelGxRA[3];
	float *imgOriginalA;  cudaMallocHost ( &imgOriginalA, memoria ) ;
	float *hImgGyA;  cudaMallocHost ( &hImgGyA, memoria ) ;
*/
	/*Inicialización*/
	//init_array(imgOriginal, hKernelGxC, hKernelGxR, SIZE);
	//init_array(imgOriginalA, hKernelGxCA, hKernelGxRA, SIZE);

	/*Kernel sobel*/
	sobelpl_cudaSYN(SIZE, GPU);
	sobelpl_cudaASYN(SIZE, GPU);
	
	
	/*Frees*/
	TimerDestroy( tTotal );
	TimerDestroy( tTotalA );
	TimerDestroy( tComp );
	TimerDestroy( tComm );

	return 0;
}






















