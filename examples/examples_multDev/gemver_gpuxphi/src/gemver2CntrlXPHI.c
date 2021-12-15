#define _DEFAULT_SOURCE
//#define USE_MKLBLAS
#include <omp.h>
#include <hitmap.h>
#include <math.h>
//#include <mkl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "timer.h"

#include "CAL.h"
#include "CAL_CntrlXPHI.h"
#include "CAL_KernelXPHI.h"
#include "CAL_KernelLibGPU.h"
//#include "CAL_AttachXPHI.h"
//#include "../../../cal/xphi/CAL_AttachXPHI.h"
#include <mkl.h>

#include <cuda_runtime.h>
#include "cublas_v2.h"

hit_tileNewType( float );


/*double  mainClock;
double  initClock;
double  sequentialClock;*/
Timer tTotal;
Timer tKernel;
Timer tPre;
Timer tIni;
Timer tEnd;

Timer tKernel_scopy1;
Timer tKernel_sger2;
Timer tKernel_sger3;
Timer tKernel_scopy4;
Timer tKernel_sgemv5;
Timer tKernel_sgemv6;


double alpha = 1.0;
double beta = 1.0;

//#define SIZE 16

//#ifdef DECLS_FLOAT
float check_matrix(HitTile_float a, int rows, int columns)
/*#else
double check_matrix(double* a, int rows, int columns)
#endif*/
{
    int i, j;
//#ifdef DECLS_FLOAT
    float cumsum = 0;
/*#else
    double cumsum = 0;
#endif*/

       //for (i=0; i<rows; i++) {
        for (j=0; j<columns; j++) {
            //cumsum += a[(i*columns)+j];
            cumsum += hit_tileElemAt(a, 1, j);
        }
    //} 
    return cumsum;
}


int Hit_Op_Tran(CALCntrl* cntrl)
{
    int result;
    switch(cntrl->type){
	case CAL_CNTRL_TYPE_XPHI:
	    result = (int)CblasTrans;
	    break;
	case CAL_CNTRL_TYPE_LIBGPU:
	    result = (int)CUBLAS_OP_T;
            break;
    }
    return result;
}

int Hit_Op_Notr(CALCntrl* cntrl)
{
    int result;
    switch(cntrl->type){
	case CAL_CNTRL_TYPE_XPHI:
	    result = (int)CblasNoTrans;
	    break;
	case CAL_CNTRL_TYPE_LIBGPU:
	    result = (int)CUBLAS_OP_N;
            break;
    }
    return result;
}

/*extern "C" void cblas_scopy (const MKL_INT n, const float *x, const MKL_INT incx, float *y, const MKL_INT incy);
extern "C" void cblas_sgemv (const CBLAS_LAYOUT Layout, const CBLAS_TRANSPOSE trans, const MKL_INT m, const MKL_INT n, const float alpha, const float *a, const MKL_INT lda, const float *x, const MKL_INT incx, const float beta, float *y, const MKL_INT incy);
extern "C" void cblas_sger (const CBLAS_LAYOUT Layout, const MKL_INT m, const MKL_INT n, const float alpha, const float *x, const MKL_INT incx, const float *y, const MKL_INT incy, float *a, const MKL_INT lda);
extern "C" void cblas_sgemm (const CBLAS_LAYOUT Layout, const CBLAS_TRANSPOSE transa, const CBLAS_TRANSPOSE transb, const MKL_INT m, const MKL_INT n, const MKL_INT k, const float alpha, const float *a, const MKL_INT lda, const float *b, const MKL_INT ldb, const float beta, float *c, const MKL_INT ldc);*/

/*extern "C" cublasStatus_t cublasScopy(cublasHandle_t handle, int n, const float *x, int incx, float *y, int incy);
extern "C" cublasStatus_t cublasSgemv(cublasHandle_t handle, cublasOperation_t trans, int m, int n, const float *alpha, const float *A, int lda, const float *x, int incx, const float *beta, float *y, int incy);
extern "C" cublasStatus_t  cublasSger(cublasHandle_t handle, int m, int n, const float *alpha, const float *x, int incx, const float *y, int incy, float *A, int lda);
extern "C" cublasStatus_t cublasSgemm(cublasHandle_t handle, cublasOperation_t transa, cublasOperation_t transb, int m, int n, int k, const float *alpha, const float *A, int lda, const float *B, int ldb, const float *beta, float *C, int ldc);*/

CAL_KERNEL_XPHI(scopy, 5, IN, HitTile_float*, n_tile, IN, HitTile_float*, x, IN, HitTile_float*, incx_tile, IO, HitTile_float*, y, IN, HitTile_float*, incy_tile)
{
	int n    = (int)hit_tileElemAtNoStride( n_tile, 1, 0 );
	int incx = (int)hit_tileElemAtNoStride( incx_tile, 1, 0 );
	int incy = (int)hit_tileElemAtNoStride( incy_tile, 1, 0 );
	/*printf("N value %i\n", n);
	printf("INCX value %i\n", incx);
	printf("INCY value %i\n", incy);*/
	//#pragma vector aligned
	//#pragma ivdep
	cblas_scopy(n, x.data, incx, y.data, incy);
}

CAL_KERNEL_XPHI(sgemv, 11, IN, HitTile_float*, trans_tile, IN, HitTile_float*, m_tile, IN, HitTile_float*, n_tile, IN, HitTile_float*, alpha_tile, IN, HitTile_float*, a, IN, HitTile_float*, lda_tile, IN, HitTile_float*, x, IN, HitTile_float*, incx_tile, IN, HitTile_float*, beta_tile, IO, HitTile_float*, y, IN, HitTile_float*, incy_tile)
{
	int trans   = (int)hit_tileElemAtNoStride(trans_tile, 1, 0);
	int m       = (int)hit_tileElemAtNoStride(m_tile, 1, 0);
	int n       = (int)hit_tileElemAtNoStride(n_tile, 1, 0);
	float alpha = (float)hit_tileElemAtNoStride(alpha_tile, 1, 0);
	int lda     = (int)hit_tileElemAtNoStride(lda_tile, 1, 0);
	int incx    = (int)hit_tileElemAtNoStride(incx_tile, 1, 0);
	float beta  = (float)hit_tileElemAtNoStride(beta_tile, 1, 0);
	int incy    = (int)hit_tileElemAtNoStride(incy_tile, 1, 0);
	//TODO: Pasar a elemenatnostride, como en la de arriba y abajo.
	/*int trans   = (int) trans_tile.kdata[0];
	int m       = (int) m_tile.kdata[0];
	int n       = (int) n_tile.kdata[0];
	float alpha = (float) alpha_tile.kdata[0];
	int lda     = (int) lda_tile.kdata[0];
	int incx    = (int) incx_tile.kdata[0];
	float beta  = (float) beta_tile.kdata[0];
	int incy    = (int) incy_tile.kdata[0];*/
	//float ve[m-100]; //Sin esta linea (y m), casca con "offload error: process on the device 0 was terminated by signal 11 (SIGSEGV)"
	float ve[m]; //Sin esta linea (y m), casca con "offload error: process on the device 0 was terminated by signal 11 (SIGSEGV)"
	#pragma vector aligned
	#pragma ivdep
#ifdef __cplusplus
	cblas_sgemv(CblasRowMajor, CBLAS_TRANSPOSE(trans), m, n, alpha, a.data, lda, x.data, incx, beta, y.data, incy); //y = αA'x + βy (matrix A' * vector x)
#else
	cblas_sgemv(CblasRowMajor, (CBLAS_TRANSPOSE)trans, m, n, alpha, a.data, lda, x.data, incx, beta, y.data, incy); //y = αA'x + βy (matrix A' * vector x)
#endif
}

CAL_KERNEL_XPHI(sger, 9, IN, HitTile_float*, m_tile, IN, HitTile_float*, n_tile, IN, HitTile_float*, alpha_tile, IN, HitTile_float*, x, IN, HitTile_float*, incx_tile, IN, HitTile_float*, y, IN, HitTile_float*, incy_tile, IO, HitTile_float*, a, IN, HitTile_float*, lda_tile)
{
        //Llamada original: CAL_CntrlLaunch(comm, sger, threads, 9, M, N, 1.0, &vectorU1, 1, &vectorV1, 1, &matrixB, N); //B = u1*v1' + B
	int m     =   (int)hit_tileElemAtNoStride( m_tile, 1, 0 );
	int n     =   (int)hit_tileElemAtNoStride( n_tile, 1, 0 );
	float alpha = (float)hit_tileElemAtNoStride( alpha_tile, 1, 0 );
        int incx  =   (int)hit_tileElemAtNoStride( incx_tile, 1, 0 );
        int incy  =   (int)hit_tileElemAtNoStride( incy_tile, 1, 0 );
        int lda   =   (int)hit_tileElemAtNoStride( lda_tile, 1, 0 );
        //int lda   =   (int)lda_tile.data[0];
	//printf("sger parameters: m = %i, n = %i, alpha = %f, incx = %i, incy = %i, lda = %i\n", m, n, alpha, incx, incy, lda); fflush(stdout);
	//float ve[m-100]; //Sin esta linea (y m), casca con "offload error: process on the device 0 was terminated by signal 11 (SIGSEGV)"
	float ve[m]; //Sin esta linea (y m), casca con "offload error: process on the device 0 was terminated by signal 11 (SIGSEGV)"
	#pragma vector aligned
	#pragma ivdep
	cblas_sger(CblasRowMajor, m, n, alpha, x.data, incx, y.data, incy, a.data, lda); //A = α*x*y' + A
}
/*
CAL_KERNEL_XPHI(sgemm, 13, IN, HitTile_float*, transa_tile, IN, HitTile_float*, transb_tile, IN, HitTile_float*, m_tile, IN, HitTile_float*, n_tile, IN, HitTile_float*, k_tile, IN, HitTile_float*, alpha_tile, IN, HitTile_float*, a, IN, HitTile_float*, lda_tile, IN, HitTile_float*, b, IN, HitTile_float*, ldb_tile, IN, HitTile_float*, beta_tile, IO, HitTile_float*, c, IN, HitTile_float*, ldc_tile)
{
	int transa  =   (int)hit_tileElemAtNoStride( transa_tile, 1, 0 );
	int transb  =   (int)hit_tileElemAtNoStride( transb_tile, 1, 0 );
	int m       =   (int)hit_tileElemAtNoStride( m_tile, 1, 0 );
	int n       =   (int)hit_tileElemAtNoStride( n_tile, 1, 0 );
	int k       =   (int)hit_tileElemAtNoStride( k_tile, 1, 0 );
	float alpha = (float)hit_tileElemAtNoStride( alpha_tile, 1, 0 );
        int lda   =   (int)hit_tileElemAtNoStride( lda_tile, 1, 0 );
        int ldb   =   (int)hit_tileElemAtNoStride( ldb_tile, 1, 0 );
	float beta = (float)hit_tileElemAtNoStride( beta_tile, 1, 0 );
        int ldc   =   (int)hit_tileElemAtNoStride( ldc_tile, 1, 0 );
	#pragma vector aligned
	#pragma ivdep
#ifdef __cplusplus
	cblas_sgemm(CblasRowMajor, CBLAS_TRANSPOSE(transa), CBLAS_TRANSPOSE(transb), m, n, k, alpha, a.data, lda, b.data, ldb, beta, c.data, ldc); //C = αA'*B' + βC,
#else
	cblas_sgemm(CblasRowMajor, (CBLAS_TRANSPOSE)transa, (CBLAS_TRANSPOSE)transb, m, n, k, alpha, a.data, lda, b.data, ldb, beta, c.data, ldc); //C = αA'*B' + βC,
#endif
}*/

//CAL_KERNEL_GPU_CHAR_STATIC(scopy, 2, full, low, low);
CAL_KERNEL_LIBGPU(scopy, 5, IN, HitTile_float*, n_tile, IN, HitTile_float*, x, IN, HitTile_float*, incx_tile, IO, HitTile_float*, y, IN, HitTile_float*, incy_tile)
{
	/*int n    = (int)hit_tileElemAtNoStride( n_tile, 1, 0 );
	int incx = (int)hit_tileElemAtNoStride( incx_tile, 1, 0 );
	int incy = (int)hit_tileElemAtNoStride( incy_tile, 1, 0 );*/
	int n    = (int) n_tile.tdata[0];
	int incx = (int) incx_tile.tdata[0];
	int incy = (int) incy_tile.tdata[0];
	//printf("Funcion scopy GPU. n = %i, incx = %i, incy = %i\n", n, incx, incy);

	cublasScopy(comm->handleCUBLAS, n, x.kdata, incx, y.kdata, incy);
}

//CAL_KERNEL_GPU_CHAR_STATIC(sgemv, 2, full, low, low);
CAL_KERNEL_LIBGPU(sgemv, 11, IN, HitTile_float*, trans_tile, IN, HitTile_float*, m_tile, IN, HitTile_float*, n_tile, IN, HitTile_float*, alpha_tile, IN, HitTile_float*, a, IN, HitTile_float*, lda_tile, IN, HitTile_float*, x, IN, HitTile_float*, incx_tile, IN,  HitTile_float*, beta_tile, IO, HitTile_float*, y, IN, HitTile_float*, incy_tile)
{
	//Llamada original: CAL_CntrlLaunch(comm, sgemv, threads, 11, CblasTrans, M, N, beta, &matrixB, M, &vectorY, 1, 1.0, &vectorX, 1); //x = βB'y + x (matrix B' * vector y)
	int trans   = (int) trans_tile.tdata[0];
	int m       = (int) m_tile.tdata[0];
	int n       = (int) n_tile.tdata[0];
	float alpha = (float) alpha_tile.tdata[0];
	int lda     = (int) lda_tile.tdata[0];
	int incx    = (int) incx_tile.tdata[0];
	float beta  = (float) beta_tile.tdata[0];
	int incy    = (int) incy_tile.tdata[0];
#ifdef __cplusplus
	cublasSgemv(comm->handleCUBLAS, cublasOperation_t(trans), m, n, &alpha, a.kdata, lda, x.kdata, incx, &beta, y.kdata, incy); //y = αA'x + βy (matrix A' * vector x)
#else
	cublasSgemv(comm->handleCUBLAS, (cublasOperation_t)trans, m, n, &alpha, a.kdata, lda, x.kdata, incx, &beta, y.kdata, incy); //y = αA'x + βy (matrix A' * vector x)
#endif
}

//CAL_KERNEL_GPU_CHAR_STATIC(sger, 2, full, low, low);
CAL_KERNEL_LIBGPU(sger, 9, IN, HitTile_float*, m_tile, IN, HitTile_float*, n_tile, IN, HitTile_float*, alpha_tile, IN, HitTile_float*, x, IN, HitTile_float*, incx_tile, IN, HitTile_float*, y, IN, HitTile_float*, incy_tile, IO, HitTile_float*, a, IN, HitTile_float*, lda_tile)
{
        //Llamada original: CAL_CntrlLaunch(comm, sger, threads, 9, M, N, 1.0, &vectorU1, 1, &vectorV1, 1, &matrixB, N); //B = u1*v1' + B
	int m     =   (int)m_tile.tdata[0];
	int n     =   (int)n_tile.tdata[0];
	float alpha = (float)alpha_tile.tdata[0];
        int incx  =   (int)incx_tile.tdata[0];
        int incy  =   (int)incy_tile.tdata[0];
        int lda   =   (int)lda_tile.tdata[0];
	cublasSger(comm->handleCUBLAS, m, n, &alpha, x.kdata, incx, y.kdata, incy, a.kdata, lda); //A = α*x*y' + A
}
/*
//CAL_KERNEL_GPU_CHAR_STATIC(sgemm, 2, full, low, low);
CAL_KERNEL_LIBGPU(sgemm, 13, IN, HitTile_float*, transa_tile, IN, HitTile_float*, transb_tile, IN, HitTile_float*, m_tile, IN, HitTile_float*, n_tile, IN, HitTile_float*, k_tile, IN, HitTile_float*, alpha_tile, IN, HitTile_float*, a, IN, HitTile_float*, lda_tile, IN, HitTile_float*, b, IN, HitTile_float*, ldb_tile, IN, HitTile_float*, beta_tile, IO, HitTile_float*, c, IN, HitTile_float*, ldc_tile)
{
	int transa  =   (int)transa_tile.tdata[0];
	int transb  =   (int)transb_tile.tdata[0];
	int m       =   (int)m_tile.tdata[0];
	int n       =   (int)n_tile.tdata[0];
	int k       =   (int)k_tile.tdata[0];
	float alpha = (float)alpha_tile.tdata[0];
        int lda     =   (int)lda_tile.tdata[0];
        int ldb     =   (int)ldb_tile.tdata[0];
	float beta  = (float)beta_tile.tdata[0];
        int ldc     =   (int)ldc_tile.tdata[0];
#ifdef __cplusplus
	cublasSgemm(comm->handleCUBLAS, cublasOperation_t(transa), cublasOperation_t(transb), m, n, k, &alpha, a.kdata, lda, b.kdata, ldb, &beta, c.kdata, ldc); //C = αA'*B' + βC,
#else
	cublasSgemm(comm->handleCUBLAS, (cublasOperation_t)transa, (cublasOperation_t)transb, m, n, k, &alpha, a.kdata, lda, b.kdata, ldb, &beta, c.kdata, ldc); //C = αA'*B' + βC,
#endif
}*/

/* A. INITIALIZE MATRIX */
void init_arrays(HitTile_float u1, HitTile_float u2, HitTile_float v1, HitTile_float v2,
                HitTile_float w, HitTile_float x, HitTile_float y, HitTile_float z,
                HitTile_float A, HitTile_float B, int size)
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
            hit_tileElemAt(B, 2, i, j) = hit_tileElemAt(A, 2, i, j);
        }
    }
}

double t_start, t_end;

int main(int argc, char *argv[])
{

    int rows = 0;
    int columns = 0;
//    #ifdef DEBUG
     setbuf(stdout, NULL);
     setbuf(stderr, NULL);
  //  #endif
    CAL_CntrlInit(1);

    if ( argc < 2 )
        {
        rows = 250;
        //columns = 250;
        }
    else
       {rows = atoi( argv[1] );}
    columns = rows;

    int i, j, k;

#ifdef DEBUG
printf("\n Rows: %d Columns: %d \n", rows,columns);
#endif

#ifdef USE_MKLBLAS
#pragma offload target(mic:0)
    {
    //printf("\n Venga vamos\n");
    }
#endif
    /* 2. DECLARE FULL MATRIX AND COPY */
    HitTile_float matrixA, matrixB;//, matrixC;
    HitTile_float vectorU1, vectorU2;
    HitTile_float vectorV1, vectorV2;
    HitTile_float vectorW, vectorX, vectorY, vectorZ;

    //Por alguna razón que desconozco, con hit_tileSingle falla y haciendo el procedimiento entero no :(
    /*double FLOAT_ONE = 1.0;
    double FLOAT_ZERO = 0.0;
    double FLOAT_ROWSCOLS = (double) rows*columns;
    double FLOAT_ROWS = (double) rows;
    double FLOAT_COLS = (double) columns;
    HitTile_float tile_one;       hit_tileSingle(&tile_one,      FLOAT_ONE,      float); 
    HitTile_float tile_zero;      hit_tileSingle(&tile_zero,     FLOAT_ZERO,     float); 
    HitTile_float tile_rowscols;  hit_tileSingle(&tile_rowscols, FLOAT_ROWSCOLS, float); 
    HitTile_float tile_rows;      hit_tileSingle(&tile_rows,     FLOAT_ROWS,     float); 
    HitTile_float tile_cols;      hit_tileSingle(&tile_cols,     FLOAT_COLS,     float); */
    HitTile_float tile_one; hit_tileDomain(&tile_one, float, 1, 1); hit_tileAlloc(&tile_one); hit_tileElemAt(tile_one, 1, 0) = 1.0 ;
    HitTile_float tile_zero; hit_tileDomain(&tile_zero, float, 1, 1); hit_tileAlloc(&tile_zero); hit_tileElemAt(tile_zero, 1, 0) = 0.0;
    HitTile_float tile_rowscols; hit_tileDomain(&tile_rowscols, float, 1, 1); hit_tileAlloc(&tile_rowscols); hit_tileElemAt(tile_rowscols, 1, 0) = (float) rows*columns;
    HitTile_float tile_rows; hit_tileDomain(&tile_rows, float, 1, 1); hit_tileAlloc(&tile_rows); hit_tileElemAt(tile_rows, 1, 0) = (float) rows;
    HitTile_float tile_cols; hit_tileDomain(&tile_cols, float, 1, 1); hit_tileAlloc(&tile_cols); hit_tileElemAt(tile_cols, 1, 0) = (float) columns;

    TimerCreate(tTotal);
    TimerCreate(tKernel);
    TimerCreate(tPre);
    TimerCreate(tIni);
    TimerCreate(tEnd);

    TimerCreate(tKernel_scopy1);
    TimerCreate(tKernel_sger2);
    TimerCreate(tKernel_sger3);
    TimerCreate(tKernel_scopy4);
    TimerCreate(tKernel_sgemv5);
    TimerCreate(tKernel_sgemv6);

    hit_tileDomain(&matrixA, float, 2, rows, columns);
    hit_tileDomain(&matrixB, float, 2, rows, columns);
    //hit_tileDomain(&matrixC, float, 2, rows, columns);
    hit_tileDomain(&vectorU1, float, 1, columns);
    hit_tileDomain(&vectorU2, float, 1, columns);
    hit_tileDomain(&vectorV1, float, 1, columns);
    hit_tileDomain(&vectorV2, float, 1, columns);
    hit_tileDomain(&vectorW, float, 1, columns);
    hit_tileDomain(&vectorX, float, 1, columns);
    hit_tileDomain(&vectorY, float, 1, columns);
    hit_tileDomain(&vectorZ, float, 1, columns);

    hit_tileAlloc(&matrixA);
    hit_tileAlloc(&matrixB); //Quitar
    //hit_tileAlloc(&matrixC); //Quitar
    hit_tileAlloc(&vectorU1);
    hit_tileAlloc(&vectorU2);
    hit_tileAlloc(&vectorV1);
    hit_tileAlloc(&vectorV2);
    hit_tileAlloc(&vectorW);
    hit_tileAlloc(&vectorX);
    hit_tileAlloc(&vectorY);
    hit_tileAlloc(&vectorZ);

    CALCntrl comm;
    CALThread threads;

    CALThreadInit(threads, 2, rows, columns);

    init_arrays(vectorU1, vectorU2, vectorV1, vectorV2, vectorW, vectorX, vectorY, vectorZ, matrixA, matrixB, columns);
/*
#ifdef DEBUG2 
	printf("\n Matrix A\n");
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        printf("%lf ", hit_tileElemAt( matrixA, 2, i, j ));
		}
		printf("\n");
	}

	printf("\n Matrix B\n");
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        printf("%lf ", hit_tileElemAt( matrixB, 2, i, j ));
		}
		printf("\n");
	}

#endif
*/
#ifndef USE_MKLBLAS
    // Initialize the device before measuring
    cudaDeviceSynchronize();
#endif

    /* CREATE CNTRLS */
    //mainClock = omp_get_wtime();
    TimerStart(tTotal);
    TimerStart(tPre);
    
#ifdef USE_MKLBLAS
    CAL_CntrlCreate(&comm, CAL_CNTRL_XPHI, 228);
#else
    CAL_CntrlCreate(&comm, CAL_CNTRL_LIBGPU, 0);
#endif

    TimerStop(tPre);
    HitTile_float tile_op_trans; hit_tileDomain(&tile_op_trans, float, 1, 1); hit_tileAlloc(&tile_op_trans); hit_tileElemAt(tile_op_trans, 1, 0) = (float) Hit_Op_Tran(&comm);
    HitTile_float tile_op_notra; hit_tileDomain(&tile_op_notra, float, 1, 1); hit_tileAlloc(&tile_op_notra); hit_tileElemAt(tile_op_notra, 1, 0) = (float) Hit_Op_Notr(&comm);
    TimerContinue(tPre);

    CAL_CntrlAttach  (&comm, (HitTile*)&matrixA);
    //CAL_CntrlInternal(&comm, (HitTile*)&matrixB); //No está implementado para XeonPhi.
    CAL_CntrlAttach  (&comm, (HitTile*)&matrixB);
    //CAL_CntrlAttach  (&comm, (HitTile*)&matrixC);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorU1);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorU2);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorV1);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorV2);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorW);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorX);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorY);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorZ);

    CAL_CntrlAttach  (&comm, (HitTile*)&tile_one);
    CAL_CntrlAttach  (&comm, (HitTile*)&tile_zero);
    CAL_CntrlAttach  (&comm, (HitTile*)&tile_rowscols);
    CAL_CntrlAttach  (&comm, (HitTile*)&tile_rows);
    CAL_CntrlAttach  (&comm, (HitTile*)&tile_cols);
    CAL_CntrlAttach  (&comm, (HitTile*)&tile_op_trans);
    CAL_CntrlAttach  (&comm, (HitTile*)&tile_op_notra);

#ifndef USE_MKLBLAS
    cudaDeviceSynchronize();
#endif
    TimerStop(tPre);

#ifndef USE_MKLBLAS
    TimerStart(tIni);
    // Se fuerza el movimiento de los datos a la GPU sólo para mediciones de tiempo
    CAL_CntrlGPUForceCntrlToHost (&comm, &matrixA);
    CAL_CntrlGPUForceCntrlToHost (&comm, &vectorU1);
    CAL_CntrlGPUForceCntrlToHost (&comm, &vectorU2);
    CAL_CntrlGPUForceCntrlToHost (&comm, &vectorV1);
    CAL_CntrlGPUForceCntrlToHost (&comm, &vectorV2);
    CAL_CntrlGPUForceCntrlToHost (&comm, &vectorW);
    CAL_CntrlGPUForceCntrlToHost (&comm, &vectorX);
    CAL_CntrlGPUForceCntrlToHost (&comm, &vectorY);
    CAL_CntrlGPUForceCntrlToHost (&comm, &vectorZ);
    cudaDeviceSynchronize();
    TimerStop(tIni);
#endif

    TimerStart(tKernel);
    int M = rows; int N = columns;
#ifdef DEBUG
    printf("Lanzando kernel 1\n"); fflush(stdout);
#endif
    TimerStart(tKernel_scopy1);
    CAL_CntrlLaunch(comm, scopy, threads, 5, &tile_rowscols, &matrixA, &tile_one, &matrixB, &tile_one);
    TimerStop(tKernel_scopy1);
#ifdef DEBUG
    printf("Lanzando kernel 2\n"); fflush(stdout);
#endif
    TimerStart(tKernel_sger2);
    CAL_CntrlLaunch(comm, sger, threads, 9, &tile_rows, &tile_cols, &tile_one, &vectorU1, &tile_one, &vectorV1, &tile_one, &matrixB, &tile_cols); //B = u1*v1' + B
    TimerStop(tKernel_sger2);
#ifdef DEBUG
    printf("Lanzando kernel 3\n"); fflush(stdout);
#endif
    TimerStart(tKernel_sger3);
    CAL_CntrlLaunch(comm, sger, threads, 9, &tile_rows, &tile_cols, &tile_one, &vectorU2, &tile_one, &vectorV2, &tile_one, &matrixB, &tile_rows); //B = u2*v2' + B
    TimerStop(tKernel_sger3);
#ifdef DEBUG
    printf("Lanzando kernel 4\n"); fflush(stdout);
#endif
    TimerStart(tKernel_scopy4);
    CAL_CntrlLaunch(comm, scopy, threads, 5, &tile_cols, &vectorZ, &tile_one, &vectorX, &tile_one); //Copy vector Z to vector X
    TimerStop(tKernel_scopy4);
#ifdef DEBUG
    printf("Lanzando kernel 5\n"); fflush(stdout);
#endif
    TimerStart(tKernel_sgemv5);
    CAL_CntrlLaunch(comm, sgemv, threads, 11, &tile_op_trans, &tile_rows, &tile_cols, &tile_one, &matrixB, &tile_rows, &vectorY, &tile_one, &tile_one, &vectorX, &tile_one); //x = βB'y + x (matrix B' * vector y)
    TimerStop(tKernel_sgemv5);

/*#ifdef USE_MKLBLAS
    CAL_CntrlLaunch(comm, sgemv, threads, 11, CblasTrans, &tile_rows, &tile_cols, beta, &matrixB, &tile_rows, &vectorY, &tile_one, &tile_one, &vectorX, &tile_one); //x = βB'y + x (matrix B' * vector y)
#else
    CAL_CntrlLaunch(comm, sgemv, threads, 11, CUBLAS_OP_T, M, N, beta, &matrixB, M, &vectorY, 1, 1.0, &vectorX, 1); //x = βB'y + x (matrix B' * vector y)
#endif*/
#ifdef DEBUG
    printf("Lanzando kernel 6\n"); fflush(stdout);
#endif
    TimerStart(tKernel_sgemv6);
    CAL_CntrlLaunch(comm, sgemv, threads, 11, &tile_op_notra, &tile_rows, &tile_cols, &tile_one, &matrixB, &tile_rows, &vectorX, &tile_one, &tile_zero, &vectorW, &tile_one);//w = αBx (matrix B * vector x) 
    TimerStop(tKernel_sgemv6);

/*#ifdef USE_MKLBLAS
    CAL_CntrlLaunch(comm, sgemv, threads, 11, CblasNoTrans, M, N, alpha, &matrixB, M, &vectorX, 1, 0.0, &vectorW, 1);//w = αBx (matrix B * vector x) 
#else
    CAL_CntrlLaunch(comm, sgemv, threads, 11, CUBLAS_OP_N, M, N, alpha, &matrixB, M, &vectorX, 1, 0.0, &vectorW, 1);//w = αBx (matrix B * vector x) 
#endif
*/
#ifndef USE_MKLBLAS
    cudaDeviceSynchronize();
#endif
    TimerStop(tKernel);

    TimerStart(tEnd);
    CAL_CntrlDetach(&comm, (HitTile*)&vectorW);
#ifndef USE_MKLBLAS
    cudaDeviceSynchronize();
#endif
    TimerStop(tEnd);
    TimerStop(tTotal);

#ifdef DEBUG
    printf("Destruyendo controlador\n"); fflush(stdout);
#endif
    CAL_CntrlDestroy(&comm);
#ifdef DEBUG
    printf("Controlador destruido\n"); fflush(stdout);
#endif
    //CAL_CntrlDestroyInternal(&comm, (HitTile*)&matrixB);

    /* 4. CLOCK RESULTS */
    //mainClock = omp_get_wtime() - mainClock;

    printf("verificacion = %lf", (double)check_matrix(vectorW, rows, columns));
    // TIMES
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock Pre: %lf\n", TimerGetTime(tPre) );
    printf("Clock IniCom: %lf\n", TimerGetTime(tIni) );
    //printf("Clock Inner: %lf\n", TimerGetTime(tInner) );
    printf("Clock seq: %lf\n", TimerGetTime(tKernel) );
    printf("Clock EndCom: %lf\n", TimerGetTime(tEnd) );
    printf("Clock main: %lf\n", TimerGetTime(tTotal) );
    //printf("Clock main: %lf\n", mainClock ); fflush(stdout);
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock tKernel_scopy1:    %.8lf\n", TimerGetTime(tKernel_scopy1));
    printf("Clock tKernel_sger2:    %.8lf\n", TimerGetTime(tKernel_sger2));
    printf("Clock tKernel_sger3:    %.8lf\n", TimerGetTime(tKernel_sger3));
    printf("Clock tKernel_scopy4:    %.8lf\n", TimerGetTime(tKernel_scopy4));
    printf("Clock tKernel_sgemv5:    %.8lf\n", TimerGetTime(tKernel_sgemv5));
    printf("Clock tKernel_sgemv6:    %.8lf\n", TimerGetTime(tKernel_sgemv6));


/*#ifdef DEBUG2 
	printf("\n Matrix C\n");
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        printf("%lf ", hit_tileElemAt( matrixC, 2, i, j ));
		}
		printf("\n");
	}
#endif


    // Calculate NORM
    double resultado=0,suma=0;
    for ( i=0; i<rows; i++ ) {
         for ( j=0; j<columns; j++ ) {
              suma += pow(  hit_tileElemAt( matrixC, 2, i, j ) ,2);
            }
    }
    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum: %lf",suma);

    resultado=sqrt( suma );
    printf("\n Result: %lf \n",resultado);

    printf("\n ---------------------------------------------------- \n");
*/
    hit_tileFree(matrixA);
    hit_tileFree(matrixB); //Quitar
    hit_tileFree(vectorU1);
    hit_tileFree(vectorU2);
    hit_tileFree(vectorV1);
    hit_tileFree(vectorV2);
    hit_tileFree(vectorW);
    hit_tileFree(vectorX);
    hit_tileFree(vectorY);
    hit_tileFree(vectorZ);

    CAL_CntrlFinish();  

  return 0;
}


