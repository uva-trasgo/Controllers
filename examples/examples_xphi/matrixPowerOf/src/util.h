#include <unistd.h>
#include <sys/time.h>

#ifdef TIME
#define IF_TIME(foo) foo;
#else
#define IF_TIME(foo)
#endif

#ifdef DECLS_FLOAT
void init_array(float* u1, float* u2, float* v1, float* v2, float* y, float* z, float* x, float* w, float* a, int columns)
#else
void init_array(double* u1, double* u2, double* v1, double* v2, double* y, double* z, double* x, double* w, double* a, int columns)
#endif
{
    int i, j;

    for (i=0; i<columns; i++) {
        u1[i] = i;
        u2[i] = (i+1)/columns/2.0;
        v1[i] = (i+1)/columns/4.0;
        v2[i] = (i+1)/columns/6.0;
        y[i] = (i+1)/columns/8.0;
        z[i] = (i+1)/columns/9.0;
        x[i] = 0.0;
        w[i] = 0.0;
        for (j=0; j<columns; j++) {
            a[(i*columns)+j] = ((double) i*j)/columns;
        }
    }
}


#ifdef DECLS_FLOAT
void print_array(int elements, float* w)
#else
void print_array(int elements, double* w)
#endif
{
    int i, j;

    for (i=0; i<elements; i++) {
        fprintf(stdout, "%0.2lf ", w[i]);
        if (i%80 == 20) fprintf(stdout, "\n");
    }
    fprintf(stdout, "\n");
}


double rtclock()
{
    //struct timezone Tzp;
    struct timeval Tp;
    int stat;
    //stat = gettimeofday (&Tp, &Tzp);
    stat = gettimeofday (&Tp, NULL);
    if (stat != 0) printf("Error return from gettimeofday: %d",stat);
    return(Tp.tv_sec + Tp.tv_usec*1.0e-6);
}
