#define N 6000

#define alpha 1.0
#define beta 1.0

#ifdef DECLS_FLOAT
float A[N][N];
float B[N][N];

float x[N];
float u1[N];
float u2[N];
float v2[N];
float v1[N];
float w[N];
float y[N];
float z[N];
#else
double A[N][N];
double B[N][N];

double x[N];
double u1[N];
double u2[N];
double v2[N];
double v1[N];
double w[N];
double y[N];
double z[N];
#endif
