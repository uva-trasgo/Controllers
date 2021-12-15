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
void init_arrays(HitTile_float u1, HitTile_float u2, HitTile_float v1, HitTile_float v2,
                HitTile_float w, HitTile_float x, HitTile_float y, HitTile_float z,
                HitTile_float A, int size)
{
    int i, j;

    //for (i=0; i<N; i++) {
    for (i = 0; i < size; i++) {
        hit_tileElemAt(u1, 1, i) = i;
        hit_tileElemAt(u2, 1, i) = ((i+1)/size)/2.0;
        hit_tileElemAt(v1, 1, i) = ((i+1)/size)/4.0;
        hit_tileElemAt(v2, 1, i) = ((i+1)/size)/6.0;
        hit_tileElemAt(y, 1, i)  = ((i+1)/size)/8.0;
        hit_tileElemAt(z, 1, i)  = ((i+1)/size)/9.0;
        hit_tileElemAt(x, 1, i)  = 0.0;
        hit_tileElemAt(w, 1, i)  = 0.0;
        for (j = 0; j < size; j++) {
            hit_tileElemAt(A, 2, i, j) = ((float) i*j)/size;
        }
    }
}

int main(int argc, char *argv[]) {

	if ( argc < 2 ) {
		int rows = 250;
		//int columns = 250;
	}
	int rows = atoi( argv[1] );
	int columns = rows;

	int i, j, k;

#ifdef DEBUG
	printf("\n Rows: %d Columns: %d \n", rows,columns);
#endif

    /* 2. DECLARE FULL MATRIX AND COPY */
	HitTile_float matrixA, matrixB;
	HitTile_float vectorU1, vectorU2;
	HitTile_float vectorV1, vectorV2;
	HitTile_float vectorW, vectorX, vectorY, vectorZ;

	hit_tileDomain(&matrixA, float, 2, rows, columns);
	hit_tileDomain(&matrixB, float, 2, rows, columns);
	hit_tileDomain(&vectorU1, float, 1, columns);
	hit_tileDomain(&vectorU2, float, 1, columns);
	hit_tileDomain(&vectorV1, float, 1, columns);
	hit_tileDomain(&vectorV2, float, 1, columns);
	hit_tileDomain(&vectorW, float, 1, columns);
	hit_tileDomain(&vectorX, float, 1, columns);
	hit_tileDomain(&vectorX, float, 1, columns);
	hit_tileDomain(&vectorZ, float, 1, columns);

	hit_tileAlloc(&matrixA);
	hit_tileAlloc(&vectorU1);
	hit_tileAlloc(&vectorU2);
	hit_tileAlloc(&vectorV1);
	hit_tileAlloc(&vectorV2);
	hit_tileAlloc(&vectorW);
	hit_tileAlloc(&vectorX);
	hit_tileAlloc(&vectorY);
	hit_tileAlloc(&vectorZ);

	Timer tTotal;
	Timer tKernel;

	CALCntrl comm;
	CALThread threads;

	CALThreadInit( threads, 2, rows, columns );

	TimerCreate(tTotal);
	TimerCreate(tKernel);
	TimerCreate(tInner);

	/* INICIALIZA LAS MATRICES */
	init_arrays(vectorU1, vectorU2, vectorV1, vectorV2, vectorW, vectorX, vectorY, vectorZ, matrixA, columns);

	cudaDeviceSynchronize();
	TimerStart(tTotal);

	CAL_CntrlCreate(&comm, CAL_CNTRL_LIBGPU, 0);
	//CAL_CntrlCreate(&comm, CAL_CNTRL_XPHI, 228);
	CAL_CntrlAttach  (&comm, (HitTile*)&matrixA);
	CAL_CntrlInternal(&comm, (HitTile*)&matrixB);
	CAL_CntrlAttach  (&comm, (HitTile*)&vectorU1);
	CAL_CntrlAttach  (&comm, (HitTile*)&vectorU2);
	CAL_CntrlAttach  (&comm, (HitTile*)&vectorV1);
	CAL_CntrlAttach  (&comm, (HitTile*)&vectorV2);
	CAL_CntrlAttach  (&comm, (HitTile*)&vectorW);
	CAL_CntrlAttach  (&comm, (HitTile*)&vectorX);
	CAL_CntrlAttach  (&comm, (HitTile*)&vectorY);
	CAL_CntrlAttach  (&comm, (HitTile*)&vectorZ);

	CAL_CntrlGPUForceCntrlToHost (&comm, &matrixA);
	CAL_CntrlGPUForceCntrlToHost (&comm, &vectorU1);
	CAL_CntrlGPUForceCntrlToHost (&comm, &vectorU2);
	CAL_CntrlGPUForceCntrlToHost (&comm, &vectorV1);
	CAL_CntrlGPUForceCntrlToHost (&comm, &vectorV2);
	CAL_CntrlGPUForceCntrlToHost (&comm, &vectorW);
	CAL_CntrlGPUForceCntrlToHost (&comm, &vectorX);
	CAL_CntrlGPUForceCntrlToHost (&comm, &vectorY);
	CAL_CntrlGPUForceCntrlToHost (&comm, &vectorZ);

	// Invoke kernel
	cudaDeviceSynchronize();
	TimerStart(tKernel);

	int M = rows; int N = columns;
	CAL_CntrlLaunch(comm, scopy, threads, 5, M*N, &matrixA, 1, &matrixB, 1);
	CAL_CntrlLaunch(comm, sger, threads, 9, M, N, 1.0, &vectorU1, 1, &vectorV1, 1, &matrixB, N); //B = u1*v1' + B
	CAL_CntrlLaunch(comm, sger, threads, 9, M, N, 1.0, &vectorU2, 1, &vectorV2, 1, &matrixB, M); //B = u2*v2' + B
	CAL_CntrlLaunch(comm, scopy, threads, 5, N, &vectorZ, 1, &vectorX, 1) //Copy vector Z to vector X
	CAL_CntrlLaunch(comm, sgemv, threads, 11, CUBLAS_OP_T, M, N, beta, &matrixB, M, &vectorY, 1, 1.0, &vectorX, 1); //x = βB'y + x (matrix B' * vector y)
	CAL_CntrlLaunch(comm, sgemv, threads, 11, CUBLAS_OP_N, M, N, alpha, &matrixB, M, &vectorX, 1, 0.0, &vectorW, 1);//w = αBx (matrix B * vector x)

	CUDA_CHECK();
	cudaDeviceSynchronize();
	TimerStop(tKernel);

	// Copy result from device memory to host memory
	CAL_CntrlDetach(&comm, (HitTile*)&vectorW);
	CAL_CntrlDestroyInternal(&comm, (HitTile*)&matrixB);

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
	hit_tileFree(vectorU1);
	hit_tileFree(vectorU2);
	hit_tileFree(vectorV1);
	hit_tileFree(vectorV2);
	hit_tileFree(vectorW);
	hit_tileFree(vectorX);
	hit_tileFree(vectorY);
	hit_tileFree(vectorZ);
	TimerDestroy(tTotal);
	TimerDestroy(tKernel);
	TimerDestroy(tInner);
	return 0;
}


