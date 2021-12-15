#include "timer.h"
#include "CAL.h"
#include "CAL_KernelLibGPU.h"

#include <cuda_runtime.h>
#include "cublas_v2.h"

#define SEED 6834723

#ifndef BLOCKSIZE
#define BLOCKSIZE 32
#endif

#undef CUDA_CHECK
#define CUDA_CHECK() { cudaError_t error; if ((error = cudaGetLastError())!=cudaSuccess){ fprintf(stdout,"%s: %s\n", cudaGetErrorName(error),cudaGetErrorString(error));exit(EXIT_FAILURE); } }

Timer tInner;

hit_tileNewType( float );

double alpha = 1.0;
double beta = 1.0;

extern "C" cublasStatus_t cublasScopy(cublasHandle_t handle, int n, const float *x, int incx, float *y, int incy);
extern "C" cublasStatus_t cublasSgemv(cublasHandle_t handle, cublasOperation_t trans, int m, int n, const float *alpha, const float *A, int lda, const float *x, int incx, const float *beta, float *y, int incy);
extern "C" cublasStatus_t  cublasSger(cublasHandle_t handle, int m, int n, const float *alpha, const float *x, int incx, const float *y, int incy, float *A, int lda);
extern "C" cublasStatus_t cublasSgemm(cublasHandle_t handle, cublasOperation_t transa, cublasOperation_t transb, int m, int n, int k, const float *alpha, const float *A, int lda, const float *B, int ldb, const float *beta, float *C, int ldc);

//CAL_KERNEL_GPU_CHAR_STATIC(scopy, 2, full, low, low);
CAL_KERNEL_LIBGPU(scopy, 5, IVAL, int, n, IN, HitTile_float*, x, IVAL, int, incx, IO, HitTile_float*, y, IVAL, int, incy)
{
	cublasScopy(comm->handleCUBLAS, n, x.kdata, incx, y.kdata, incy);
}

//CAL_KERNEL_GPU_CHAR_STATIC(sgemv, 2, full, low, low);
CAL_KERNEL_LIBGPU(sgemv, 11, IVAL, int, trans, IVAL, int, m, IVAL, int, n, IVAL, float, alpha, IN, HitTile_float*, a, IVAL, int, lda, IN, HitTile_float*, x, IVAL, int, incx, IVAL, float, beta, IO, HitTile_float*, y, IVAL, int, incy)
{
#ifdef __cplusplus
	cublasSgemv(comm->handleCUBLAS, cublasOperation_t(trans), m, n, &alpha, a.kdata, lda, x.kdata, incx, &beta, y.kdata, incy); //y = αA'x + βy (matrix A' * vector x)
#else
	cublasSgemv(comm->handleCUBLAS, (cublasOperation_t)trans, m, n, &alpha, a.kdata, lda, x.kdata, incx, &beta, y.kdata, incy); //y = αA'x + βy (matrix A' * vector x)
#endif
}

//CAL_KERNEL_GPU_CHAR_STATIC(sger, 2, full, low, low);
CAL_KERNEL_LIBGPU(sger, 9, IVAL, int, m, IVAL, int, n, IVAL, float, alpha, IN, HitTile_float*, x, IVAL, int, incx, IN, HitTile_float*, y, IVAL, int, incy, IO, HitTile_float*, a, IVAL, int, lda)
{
	cublasSger(comm->handleCUBLAS, m, n, &alpha, x.kdata, incx, y.kdata, incy, a.kdata, lda); //A = α*x*y' + A
}

//CAL_KERNEL_GPU_CHAR_STATIC(sgemm, 2, full, low, low);
CAL_KERNEL_LIBGPU(sgemm, 13, IVAL, int, transa, IVAL, int, transb, IVAL, int, m, IVAL, int, n, IVAL, int, k, IVAL, float, alpha, IN, HitTile_float*, a, IVAL, int, lda, IN, HitTile_float*, b, IVAL, int, ldb, IVAL, float, beta, IO, HitTile_float*, c, IVAL, int, ldc)
{
#ifdef __cplusplus
	cublasSgemm(comm->handleCUBLAS, cublasOperation_t(transa), cublasOperation_t(transb), m, n, k, &alpha, a.kdata, lda, b.kdata, ldb, &beta, c.kdata, ldc); //C = αA'*B' + βC,
#else
	cublasSgemm(comm->handleCUBLAS, (cublasOperation_t)transa, (cublasOperation_t)transb, m, n, k, &alpha, a.kdata, lda, b.kdata, ldb, &beta, c.kdata, ldc); //C = αA'*B' + βC,
#endif
}

/* A. INITIALIZE MATRIX */
void init_arrays(HitTile_float A, int rows, int columns)
{
    int i, j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < columns; j++) {
            hit_tileElemAt(A, 2, i, j) = ((float) i*j)/rows;
        }
    }
}

int main(int argc, char *argv[]) {

	int final_power = 0;
	int rows = 0;
	int columns = 0;

	if (argc < 2){
		printf("Error. Numero de argumentos insuficiente\n");
		exit(-1);
		}
	else if( argc < 3 ) {
		final_power = atoi( argv[1] );
		rows = 250;
		columns = 250;
		}
	else if( argc < 4 ) {
		final_power = atoi( argv[1] );
		rows = atoi( argv[2] );
		columns = rows;
		}
	else if( argc >= 4 ) {
		final_power = atoi( argv[1] );
		rows = atoi( argv[2] );
		columns = atoi( argv[3] );
		}

	//int i, j, k;

#ifdef DEBUG
	printf("\n Rows: %d Columns: %d \n", rows,columns);
#endif

    /* 2. DECLARE FULL MATRIX AND COPY */
	HitTile_float matrixA; //, matrixB;

	hit_tileDomain(&matrixA, float, 2, rows, columns);
	//hit_tileDomain(&matrixB, float, 2, rows, columns);

	hit_tileAlloc(&matrixA);

	Timer tTotal;
	Timer tKernel;

	CALCntrl comm;
	CALThread threads;

	CALThreadInit( threads, 2, rows, columns );

	TimerCreate(tTotal);
	TimerCreate(tKernel);
	TimerCreate(tInner);

	/* INICIALIZA LAS MATRICES */
	init_arrays(matrixA, rows, columns);

	cudaDeviceSynchronize();
	TimerStart(tTotal);

	CAL_CntrlCreate(&comm, CAL_CNTRL_LIBGPU, 0);
	//CAL_CntrlCreate(&comm, CAL_CNTRL_XPHI, 228);
	CAL_CntrlAttach  (&comm, (HitTile*)&matrixA);
	//CAL_CntrlInternal(&comm, (HitTile*)&matrixB);

	CAL_CntrlGPUForceCntrlToHost (&comm, &matrixA);

	// Invoke kernel
	cudaDeviceSynchronize();
	TimerStart(tKernel);

	//int M = rows; int N = columns;
	int power;
	for (power = 1; power >= final_power; power++)
		CAL_CntrlLaunch(comm, sgemm, threads, 13, CUBLAS_OP_N, CUBLAS_OP_N, rows, columns, columns, 1.0, &matrixA, columns, &matrixA, columns, 0.0, &matrixA, rows); //C = αAB + βC
		//CAL_CntrlLaunch(comm, sgemm, threads, 13, CUBLAS_OP_N, CUBLAS_OP_N, rows, columns, columns, 1.0, &matrixA, lda, &matrixA, ldb, 0.0, &matrixA, ldc); //C = αAB + βC

	CUDA_CHECK();
	cudaDeviceSynchronize();
	TimerStop(tKernel);

	// Copy result from device memory to host memory
	CAL_CntrlDetach(&comm, (HitTile*)&matrixA);

	cudaDeviceSynchronize();
	TimerStop(tTotal);

	CAL_CntrlDestroy(&comm);

#ifdef NDEBUG
	printf("SIZE %d\n", rows);
	printf("CLOCK Main %.8lf\n", TimerGetTime(tTotal) );
	printf("CLOCK Kernels %.8lf\n", TimerGetTime(tKernel) );
	printf("CLOCK Inner %.8lf\n", TimerGetTime(tInner) );
#else
	printf("Ejecutado en modo depuracion.\n");
	printf("Tamaño de la matriz: %d x %d\n",rows,columns);
	printf("Tiempo total: %.8lf\n",TimerGetTime(tTotal));
	printf("Tiempo kernel: %.8lf\n",TimerGetTime(tKernel));
	/*for (int i=0; i<SIZE; i++)
		for (int j=0; j<SIZE; j++) 
			if ( fabs( hit_tileElemAt(C, 2, i, j) - 18.345 * 1.14 * SIZE ) > 0.001 )
				fprintf(stderr, "ERROR: Resultado (%d,%d) = %lf\n", i, j,
					hit_tileElemAt(C, 2, i, j) );*/

#endif

	hit_tileFree(matrixA);
	TimerDestroy(tTotal);
	TimerDestroy(tKernel);
	TimerDestroy(tInner);
	return 0;
}


