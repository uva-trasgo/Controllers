#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <mkl.h>

#include "decls.h"
#include "util.h"

main()
{
    double t_start, t_end;
    int i, j;
    int M=N;
#ifdef DECLS_FLOAT
    float *a, *b;
#else
    double *a, *b;
#endif

    //init_array();
    init_array(u1, u2, v1, v2, y, z, x, w, a, N);

#ifdef DECLS_FLOAT
    a = (float *) malloc(sizeof(float)*M*M);
    b = (float *) malloc(sizeof(float)*M*M);
#else
    a = (double *) malloc(sizeof(double)*M*M);
    b = (double *) malloc(sizeof(double)*M*M);
#endif

    for( i=0; i<M; i++){
        for( j=0; j<N; j++){
            a[i*M+j]= A[i][j]; 
        }
    }

    IF_TIME(t_start = rtclock());

#ifdef DECLS_FLOAT
    cblas_scopy(M*N, a, 1, b, 1);
    cblas_sger(CblasRowMajor, M, N, 1.0, u1, 1, v1, 1, b, N);
    cblas_sger(CblasRowMajor, M, N, 1.0, u2, 1, v2, 1, b, M);
    cblas_scopy(N, z, 1, x, 1);
    cblas_sgemv(CblasRowMajor,CblasTrans,M,N,beta,b,M,y,1,1.0,x,1);
    cblas_sgemv(CblasRowMajor,CblasNoTrans,M,N,alpha,b,M,x,1,0.0,w,1);
#else
    cblas_dcopy(M*N, a, 1, b, 1);
    cblas_dger(CblasRowMajor, M, N, 1.0, u1, 1, v1, 1, b, N);
    cblas_dger(CblasRowMajor, M, N, 1.0, u2, 1, v2, 1, b, M);
    cblas_dcopy(N, z, 1, x, 1);
    cblas_dgemv(CblasRowMajor,CblasTrans,M,N,beta,b,M,y,1,1.0,x,1);
    cblas_dgemv(CblasRowMajor,CblasNoTrans,M,N,alpha,b,M,x,1,0.0,w,1);
#endif

    IF_TIME(t_end = rtclock());
    IF_TIME(fprintf(stderr, "%0.6lfs\n", t_end - t_start));

#ifdef TEST
    //print_array();
    print_array(N, w);
#endif

    return 0;
}
