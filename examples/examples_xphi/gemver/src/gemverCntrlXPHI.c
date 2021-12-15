#define _DEFAULT_SOURCE
#include <omp.h>
#include <hitmap.h>
#include <math.h>
//#include <mkl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "CAL.h"
#include "CAL_CntrlXPHI.h"
#include "CAL_KernelXPHI.h"
//#include "CAL_AttachXPHI.h"
//#include "../../../cal/xphi/CAL_AttachXPHI.h"
#include <mkl.h>

hit_tileNewType( float );


double  mainClock;
double  initClock;
double  sequentialClock;

double alpha = 1.0;
double beta = 1.0;

#define SIZE 16

/*extern "C" void cblas_scopy (const MKL_INT n, const float *x, const MKL_INT incx, float *y, const MKL_INT incy);
extern "C" void cblas_sgemv (const CBLAS_LAYOUT Layout, const CBLAS_TRANSPOSE trans, const MKL_INT m, const MKL_INT n, const float alpha, const float *a, const MKL_INT lda, const float *x, const MKL_INT incx, const float beta, float *y, const MKL_INT incy);
extern "C" void cblas_sger (const CBLAS_LAYOUT Layout, const MKL_INT m, const MKL_INT n, const float alpha, const float *x, const MKL_INT incx, const float *y, const MKL_INT incy, float *a, const MKL_INT lda);
extern "C" void cblas_sgemm (const CBLAS_LAYOUT Layout, const CBLAS_TRANSPOSE transa, const CBLAS_TRANSPOSE transb, const MKL_INT m, const MKL_INT n, const MKL_INT k, const float alpha, const float *a, const MKL_INT lda, const float *b, const MKL_INT ldb, const float beta, float *c, const MKL_INT ldc);*/

CAL_KERNEL_XPHI(scopy, 5, IVAL, int, n, IN, HitTile_float*, x, IVAL, int, incx, IO, HitTile_float*, y, IVAL, int, incy)
{
	#pragma vector aligned
	#pragma ivdep
	cblas_scopy(n, x.data, incx, y.data, incy);
}

CAL_KERNEL_XPHI(sgemv, 11, IVAL, int, trans, IVAL, int, m, IVAL, int, n, IVAL, float, alpha, IN, HitTile_float*, a, IVAL, int, lda, IN, HitTile_float*, x, IVAL, int, incx, IVAL, float, beta, IO, HitTile_float*, y, IVAL, int, incy)
{
	#pragma vector aligned
	#pragma ivdep
#ifdef __cplusplus
	cblas_sgemv(CblasRowMajor, CBLAS_TRANSPOSE(trans), m, n, alpha, a.data, lda, x.data, incx, beta, y.data, incy); //y = αA'x + βy (matrix A' * vector x)
#else
	cblas_sgemv(CblasRowMajor, (CBLAS_TRANSPOSE)trans, m, n, alpha, a.data, lda, x.data, incx, beta, y.data, incy); //y = αA'x + βy (matrix A' * vector x)
#endif
}

CAL_KERNEL_XPHI(sger, 9, IVAL, int, m, IVAL, int, n, IVAL, float, alpha, IN, HitTile_float*, x, IVAL, int, incx, IN, HitTile_float*, y, IVAL, int, incy, IO, HitTile_float*, a, IVAL, int, lda)
{
	#pragma vector aligned
	#pragma ivdep
	cblas_sger(CblasRowMajor, m, n, alpha, x.data, incx, y.data, incy, a.data, lda); //A = α*x*y' + A
}

CAL_KERNEL_XPHI(sgemm, 13, IVAL, int, transa, IVAL, int, transb, IVAL, int, m, IVAL, int, n, IVAL, int, k, IVAL, float, alpha, IN, HitTile_float*, a, IVAL, int, lda, IN, HitTile_float*, b, IVAL, int, ldb, IVAL, float, beta, IO, HitTile_float*, c, IVAL, int, ldc)
{
	#pragma vector aligned
	#pragma ivdep
#ifdef __cplusplus
	cblas_sgemm(CblasRowMajor, CBLAS_TRANSPOSE(transa), CBLAS_TRANSPOSE(transb), m, n, k, alpha, a.data, lda, b.data, ldb, beta, c.data, ldc); //C = αA'*B' + βC,
#else
	cblas_sgemm(CblasRowMajor, (CBLAS_TRANSPOSE)transa, (CBLAS_TRANSPOSE)transb, m, n, k, alpha, a.data, lda, b.data, ldb, beta, c.data, ldc); //C = αA'*B' + βC,
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

double t_start, t_end;

int main(int argc, char *argv[])
{

//    #ifdef DEBUG
     setbuf(stdout, NULL);
     setbuf(stderr, NULL);
  //  #endif

    CAL_CntrlInit(1);

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

#pragma offload target(mic:0)
{
//printf("\n Venga vamos\n");
}
    
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

    CALCntrl comm;
    CALThread threads;

    CALThreadInit(threads, 2, rows, columns);

    init_arrays(vectorU1, vectorU2, vectorV1, vectorV2, vectorW, vectorX, vectorY, vectorZ, matrixA, columns);
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

    /* CREATE CNTRLS */
    mainClock = omp_get_wtime();
    

    CAL_CntrlCreate(&comm, CAL_CNTRL_XPHI, 228);
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

    int M = rows; int N = columns;
    CAL_CntrlLaunch(comm, scopy, threads, 5, M*N, &matrixA, 1, &matrixB, 1);
    CAL_CntrlLaunch(comm, sger, threads, 9, M, N, 1.0, &vectorU1, 1, &vectorV1, 1, &matrixB, N); //B = u1*v1' + B
    CAL_CntrlLaunch(comm, sger, threads, 9, M, N, 1.0, &vectorU2, 1, &vectorV2, 1, &matrixB, M); //B = u2*v2' + B
    CAL_CntrlLaunch(comm, scopy, threads, 5, N, &vectorZ, 1, &vectorX, 1) //Copy vector Z to vector X
    CAL_CntrlLaunch(comm, sgemv, threads, 11, CblasTrans, M, N, beta, &matrixB, M, &vectorY, 1, 1.0, &vectorX, 1); //x = βB'y + x (matrix B' * vector y)
    CAL_CntrlLaunch(comm, sgemv, threads, 11, CblasNoTrans, M, N, alpha, &matrixB, M, &vectorX, 1, 0.0, &vectorW, 1);//w = αBx (matrix B * vector x) 

    CAL_CntrlDetach(&comm, (HitTile*)&vectorW);
    CAL_CntrlDestroyInternal(&comm, (HitTile*)&matrixB);

    CAL_CntrlDestroy(&comm);


    /* 4. CLOCK RESULTS */
    mainClock = omp_get_wtime() - mainClock;

    // TIMES
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock main: %lf\n", mainClock );



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


