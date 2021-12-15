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
}

/*CAL_KERNEL_XPHI(sgemm, 13, IVAL, int, transa, IVAL, int, transb, IVAL, int, m, IVAL, int, n, IVAL, int, k, IVAL, float, alpha, IN, HitTile_float*, a, IVAL, int, lda, IN, HitTile_float*, b, IVAL, int, ldb, IVAL, float, beta, IO, HitTile_float*, c, IVAL, int, ldc)
{
	#pragma vector aligned
	#pragma ivdep
#ifdef __cplusplus
	cblas_sgemm(CblasRowMajor, CBLAS_TRANSPOSE(transa), CBLAS_TRANSPOSE(transb), m, n, k, alpha, a.data, lda, b.data, ldb, beta, c.data, ldc); //C = αA'*B' + βC,
#else
	cblas_sgemm(CblasRowMajor, (CBLAS_TRANSPOSE)transa, (CBLAS_TRANSPOSE)transb, m, n, k, alpha, a.data, lda, b.data, ldb, beta, c.data, ldc); //C = αA'*B' + βC,
#endif
}*/

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


double t_start, t_end;

int main(int argc, char *argv[])
{

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

//    #ifdef DEBUG
     setbuf(stdout, NULL);
     setbuf(stderr, NULL);
  //  #endif

    CAL_CntrlInit(1);

    int i, j, k;

#ifdef DEBUG
printf("\n Rows: %d Columns: %d \n", rows,columns);
#endif

#pragma offload target(mic:0)
{
//printf("\n Venga vamos\n");
}
    
    /* 2. DECLARE FULL MATRIX AND COPY */
    HitTile_float matrixA; //, matrixB;

    HitTile_float tile_one; hit_tileDomain(&tile_one, float, 1, 1); hit_tileAlloc(&tile_one); hit_tileElemAt(tile_one, 1, 0) = 1.0 ;
    HitTile_float tile_zero; hit_tileDomain(&tile_zero, float, 1, 1); hit_tileAlloc(&tile_zero); hit_tileElemAt(tile_zero, 1, 0) = 0.0;
    HitTile_float tile_rowscols; hit_tileDomain(&tile_rowscols, float, 1, 1); hit_tileAlloc(&tile_rowscols); hit_tileElemAt(tile_rowscols, 1, 0) = (float) rows*columns;
    HitTile_float tile_rows; hit_tileDomain(&tile_rows, float, 1, 1); hit_tileAlloc(&tile_rows); hit_tileElemAt(tile_rows, 1, 0) = (float) rows;
    HitTile_float tile_cols; hit_tileDomain(&tile_cols, float, 1, 1); hit_tileAlloc(&tile_cols); hit_tileElemAt(tile_cols, 1, 0) = (float) columns;

    hit_tileDomain(&matrixA, float, 2, rows, columns);
    //hit_tileDomain(&matrixB, float, 2, rows, columns);

    hit_tileAlloc(&matrixA);

    CALCntrl comm;
    CALThread threads;

    CALThreadInit(threads, 2, rows, columns);

    init_arrays(matrixA, rows, columns);
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

    HitTile_float tile_op_trans; hit_tileDomain(&tile_op_trans, float, 1, 1); hit_tileAlloc(&tile_op_trans); hit_tileElemAt(tile_op_trans, 1, 0) = (float) Hit_Op_Tran(&comm);
    HitTile_float tile_op_notra; hit_tileDomain(&tile_op_notra, float, 1, 1); hit_tileAlloc(&tile_op_notra); hit_tileElemAt(tile_op_notra, 1, 0) = (float) Hit_Op_Notr(&comm);
    CAL_CntrlAttach  (&comm, (HitTile*)&matrixA);
    //CAL_CntrlInternal(&comm, (HitTile*)&matrixB);

    CAL_CntrlAttach  (&comm, (HitTile*)&tile_one);
    CAL_CntrlAttach  (&comm, (HitTile*)&tile_zero);
    CAL_CntrlAttach  (&comm, (HitTile*)&tile_rowscols);
    CAL_CntrlAttach  (&comm, (HitTile*)&tile_rows);
    CAL_CntrlAttach  (&comm, (HitTile*)&tile_cols);
    CAL_CntrlAttach  (&comm, (HitTile*)&tile_op_trans);
    CAL_CntrlAttach  (&comm, (HitTile*)&tile_op_notra);

    //int M = rows; int N = columns;
    int power;
        for (power = 1; power >= final_power; power++)
                CAL_CntrlLaunch(comm, sgemm, threads, 13, &tile_op_notra, &tile_op_notra, &tile_rows, &tile_cols, &tile_cols, &tile_one, &matrixA, &tile_cols, &matrixA, &tile_cols, &tile_zero, &matrixA, &tile_rows); //C = αAB + βC
                //CAL_CntrlLaunch(comm, sgemm, threads, 13, CUBLAS_OP_N, CUBLAS_OP_N, rows, columns, columns, 1.0, &matrixA, lda, &matrixA, ldb, 0.0, &matrixA, ldc); //C = αAB + βC
    

    CAL_CntrlDetach(&comm, (HitTile*)&matrixA);

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

    CAL_CntrlFinish();  

  return 0;
}


