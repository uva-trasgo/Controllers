#include <stdio.h>
#include <stdlib.h>
#include <mkl.h>

#define DECLS_FLOAT
//#include "decls.h"
#include "util.h"

int main(int argc, char* argv[])
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

    double t_start, t_end;
    int i, j;
    //int M=N;
#ifdef DECLS_FLOAT
    float alpha = 1.0;
    float beta = 0.0;
    float *a; //, *b;
#else
    double alpha = 1.0;
    double beta = 0.0;
    double *a; //, *b;
#endif

    //init_array();
    for( i=0; i<rows; i++){
        for( j=0; j<columns; j++){
#ifdef DECLS_FLOAT
            a[i*columns+j]= ((float) i*j)/rows;
#else
            a[i*columns+j]= ((double) i*j)/rows;
#endif
        }
    }


#ifdef DECLS_FLOAT
    a = (float *) malloc(sizeof(float)*rows*columns);
    //b = (float *) malloc(sizeof(float)*rows*columns);
#else
    a = (double *) malloc(sizeof(double)*rows*columns);
    //b = (double *) malloc(sizeof(double)*rows*columns);
#endif

    /*for( i=0; i<M; i++){
	for( j=0; j<N; j++){
	    a[i*M+j]= A[i][j]; 
	}
    }*/

    IF_TIME(t_start = rtclock());
    /*in(u1:length(M)) \
    in(u2:length(M)) \
    in(v1:length(M)) \
    in(v2:length(M)) \
    out(w:length(M)) \
    in(x:length(M)) \
    in(y:length(M)) \
    in(z:length(M))*/

    mkl_mic_set_offload_report(2);

    #pragma offload target(mic) \
    in(rows) \
    in(columns) \
    inout(a: length(rows*columns)) \
    in(final_power)
    {
	int power;
	for (power = 1; power >= final_power; power++)
	{
            cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,rows,columns,columns,alpha,a,columns,a,columns,beta,a,rows);
	}
    }
    IF_TIME(t_end = rtclock());
    IF_TIME(fprintf(stderr, "%0.6lfs\n", t_end - t_start));

#ifdef TEST
    print_array(columns, w);
#endif

    return 0;
}

