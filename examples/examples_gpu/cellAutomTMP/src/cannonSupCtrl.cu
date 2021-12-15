#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <assert.h>
#include <hitmap.h>
#include "timer.h"
#include "CAL.h"


#ifndef BLOCKSIZE
#define BLOCKSIZE 32
#endif

#define	max(a,b)	((a)>(b)) ? (b):(a)

hit_tileNewType( float );
hit_ktileNewType( float );

int 	MATRIX_AROWS;		 
int 	MATRIX_ACOLUMNS; 
int 	MATRIX_BCOLUMNS; 
int 	gpu;

HitClock	mainClock;
HitClock	commClock;
HitClock	productClock;

///////////////////////////////////////////////////////////////////////////////
// Kernel function
///////////////////////////////////////////////////////////////////////////////
/* A. CPU Kernel charazterization */
CAL_KERNEL_GPU_CHAR_STATIC(cannonCu, 2, fixed, square, 32);

/* B. GPU Kernel implementation */
CAL_KERNEL(cannonCu, dGPU, 
		 KHitTile_float A, 
		 KHitTile_float B, 
		 KHitTile_float C
) {
	//printf("sdfsdfasgdgdfgsdfsdgsdfgsdgf\n");
	// Block positions
	int aCol = 0;
	int bRow = 0;
	int bCol = blockIdx.x * BLOCKSIZE;
	// Thread index
	int row = threadIdx.y;
	int col = threadIdx.x;

	// Each thread computes one element of Csub
	// by accumulating results into Cvalue
	float Cvalue = 0;

	
	// Loop over all the sub-matrices of A and B
	// required to compute the block sub-matrix
	for (int iter=0; 
		iter < A.origAcumCard[1] / BLOCKSIZE; 
		iter++, aCol+=BLOCKSIZE, bRow+=BLOCKSIZE )
	{
		// Shared memory used to store Asub and Bsub respectively
		__shared__ float As[BLOCKSIZE][BLOCKSIZE];
		__shared__ float Bs[BLOCKSIZE][BLOCKSIZE];

		// Load the matrices from device memory
		// to shared memory; each thread loads
		// one element of each matrix
		As[row][col] = hit_tileElemAtNoStride( A, 2, threadId.y, aCol + col );
		Bs[row][col] = hit_tileElemAtNoStride( B, 2, bRow + row, bCol + col );

		// Synchronize to make sure the matrices are loaded
		__syncthreads();

		// Multiply the two matrices together;
		// each thread computes one element
		// of the block sub-matrix
		#pragma unroll
		for (int k = 0; k < BLOCKSIZE; ++k)
			Cvalue += As[row][k] * Bs[k][col];

		// Synchronize to make sure that the preceding
		// computation is done before loading two new
		// sub-matrices of A and B in the next iteration
		__syncthreads();
	}

	// Write the block sub-matrix to device memory;
	// each thread writes one element
	hit_tileElemAtNoStride(C, 2, threadId.y, threadId.x) = hit_tileElemAtNoStride(C, 2, threadId.y, threadId.x)+ Cvalue;
}//

/* C. Defining kernel prototypes */
CAL_KERNEL_PROTO(cannonCu, 
	1, dGPU,
	3, 
	OUT, HitTile_float, A, 
	OUT, HitTile_float, B, 
	OUT, HitTile_float, C
	);



void cannonsMM( int n, int m, int p );


/* A. MAIN: INVOCATION OF CANNON'S ALGORITHM */
int main(int argc, char *argv[]){
	hit_comInit( &argc, &argv );

	// 1. Init Controllers library
	//CAL_CntrlInit(2);
	omp_set_nested(1);
	omp_set_num_threads(3);
	#pragma omp parallel 
	{
	#pragma omp single 
	{	

    // 2. Taking arguments
  	if ( argc != 4 ) {
  	     fprintf(stderr, "\nUsage: %s <size> <size> <GPU>\n", argv[0]);
  	     exit(EXIT_FAILURE);
  	}
  	
  	MATRIX_AROWS = atoi( argv[1] );		 
	MATRIX_ACOLUMNS = atoi( argv[1] );
	MATRIX_BCOLUMNS = atoi( argv[1] );
  	gpu = atoi( argv[3] );
  	
	cannonsMM( MATRIX_AROWS, MATRIX_ACOLUMNS, MATRIX_BCOLUMNS );

	//hit_comFinalize();
	CAL_CntrlFinish();
	
	return 0;
}


/* B. FUNCTION TO COMPUTE THE PRODUCT OF TWO LOCAL SUBMATRICES */
void matrixBlockProduct( HitTile_float A, HitTile_float B, HitTile_float C ) {
	int i,j,k;
	
#ifdef OPTIMIZE_PRODUCT
	double *datas[3] = { C.data, A.data, B.data };
	int rowSizes[3] ={  C.origAcumCard[1],A.origAcumCard[1],B.origAcumCard[1] };
#  ifdef OPTIMIZE_LOOPS
	int loopLimits[3] = { hit_tileDimCard(C,0), hit_tileDimCard(C,1), hit_tileDimCard(A,1) };
	for (i=0; i<loopLimits[0]; i++) {
		for (j=0; j<loopLimits[1]; j++) {
			for (k=0; k<loopLimits[2]; k++) {
#   else
	for (i=0; i<hit_tileDimCard(C,0); i++) {
		for (j=0; j<hit_tileDimCard(C,1); j++) {
			for (k=0; k<hit_tileDimCard(A,1); k++) {
#   endif
#else
	for (i=0; i<hit_tileDimCard(C,0); i++) {
		for (j=0; j<hit_tileDimCard(C,1); j++) {
			for (k=0; k<hit_tileDimCard(A,1); k++) {
#endif

#ifdef OPTIMIZE_PRODUCT
					datas[0][i*rowSizes[0]+j] =
						datas[0][i*rowSizes[0]+j] + datas[1][i*rowSizes[1]+k] * datas[2][k*rowSizes[2]+j] ;
#else
					hit_tileElemAtNoStride(C,2,i,j) = 
							hit_tileElemAtNoStride(C,2,i,j) 
							+ hit_tileElemAtNoStride(A,2,i,k) * hit_tileElemAtNoStride(B,2,k,j);
#endif
			}
		}
	}
}


/* C. INITIALIZE MATRICES */
void initMatrices( int m, int p, 
					HitTile_float tileA, HitTile_float tileB, HitTile_float tileC,
					HitLayout layA, HitLayout layB ) {
	/* 1. INIT C = 0 */
	double zero=0; 
	hit_tileFill( &tileA, &zero );
	hit_tileFill( &tileB, &zero );
	hit_tileFill( &tileC, &zero );

#if defined(READ_AB) || defined(WRITE_AB)
	HitTile_double realTileA, realTileB;
	hit_tileSelectArrayCoords( &realTileA, &tileA, hit_layShape( layA ) );
	hit_tileSelectArrayCoords( &realTileB, &tileB, hit_layShape( layB ) );
#endif

#ifdef READ_AB
#ifdef READ_BIN
	hit_tileFileRead( &realTileA, "A.in", HIT_FILE_ARRAY );
	hit_tileFileRead( &realTileB, "B.in", HIT_FILE_ARRAY );
#else
	hit_tileTextFileRead( &realTileA, "A.in.dtxt", HIT_FILE_ARRAY, HIT_FILE_FLOAT, 14, 4 );
	hit_tileTextFileRead( &realTileB, "B.in.dtxt", HIT_FILE_ARRAY, HIT_FILE_FLOAT, 14, 4 );
#endif
#else
	int i,j;

	/* 2. INIT A(i,j) = ( i*numCol + j ) % 1000 */
	hit_layForDimDomain( layA, 0, i )  
		hit_layForDimDomain( layA, 1, j )  
			hit_tileElemAtArrayCoords( tileA, 2, i,j ) = ( i * m + j ) % 1000;

	/* 3. INIT B: DIFFERENT COMPILE-TIME OPTIONS */
#ifdef B_IDENTITY
	/* 3.a. INIT B = Identity */
	hit_layForDimDomain( layB, 0, i )  
		hit_layForDimDomain( layB, 1, j )  
			hit_tileElemAtArrayCoords( tileB, 2, i,j ) = (i==j) ?  1.0 : 0.0;
#else
	/* 3.b. INIT B = Random with seed */
	srand48( 364543 );
	for (i=0; i<m; i++) {
		for (j=0; j<p; j++) {
			if ( i>=hit_layDimBegin(layB,0) && i<=hit_layDimEnd(layB,0) && j>=hit_layDimBegin(layB,1) && j<=hit_layDimEnd(layB,1) ) 
				hit_tileElemAtArrayCoords( tileB, 2, i,j ) = drand48();
			else drand48();
		}
	}
#endif
#endif
#ifdef WRITE_AB
	/* 4. WRITE A,B ON FILES */
#ifdef WRITE_BIN
	hit_tileFileWrite( &realTileA, "A.out", HIT_FILE_ARRAY );
	hit_tileFileWrite( &realTileB, "B.out", HIT_FILE_ARRAY );
#else
#ifdef WRITE_TILE_FILES
	char name[8];
	sprintf(name, "A.%d", hit_Rank );
	hit_tileTextFileWrite( &realTileA, name, HIT_FILE_TILE, HIT_FILE_FLOAT, 14, 4 );
	sprintf(name, "B.%d", hit_Rank );
	hit_tileTextFileWrite( &realTileB, name, HIT_FILE_TILE, HIT_FILE_FLOAT, 14, 4 );
#endif
	hit_tileTextFileWrite( &realTileA, "A.out.dtxt", HIT_FILE_ARRAY, HIT_FILE_FLOAT, 14, 4 );
	hit_tileTextFileWrite( &realTileB, "B.out.dtxt", HIT_FILE_ARRAY, HIT_FILE_FLOAT, 14, 4 );
#endif
#endif
}


/* D. PARALLEL CANNON'S ALGORITHM FOR MATRIX MULTIPLICATION */
void cannonsMM( int n, int m, int p )  {

  	int size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	 
	int GPU = rank; //gpu;



	// Create controller object
	CALCntrl comm;
	CAL_CntrlCreate(&comm, CAL_CNTRL_GPU, GPU );
	

	//hit_clockReset( productClock );

    /* 1. CREATE VIRTUAL TOPOLOGY */
    HitTopology topo = hit_topology( plug_topSquare );

	/* 2. DECLARE FULL MATRICES WITHOUT MEMORY */
	HitTile_float A, B, C;
	hit_tileDomain( &A, double, 2, n, m );
	hit_tileDomain( &B, double, 2, m, p );
	hit_tileDomain( &C, double, 2, n, p );

    /* 3. COMPUTE PARTITIONS */
#define LAYOUT_BALANCED
#ifdef LAYOUT_BALANCED
	HitLayout layA = hit_layout(plug_layBlocksBalance, topo, hit_tileShape( A ), 0, 0.8f );
    HitLayout layC = hit_layout(plug_layBlocksBalance, topo, hit_tileShape( C ), 0, 0.8f );
#else
	HitLayout layA = hit_layout(plug_layBlocks, topo, hit_tileShape( A ) );
    HitLayout layC = hit_layout(plug_layBlocks, topo, hit_tileShape( C ) );
#endif
	HitLayout layB = hit_layout(plug_layBlocks, topo, hit_tileShape( B ) );
	
	hit_layWrapNeighbors( &layA );
	hit_layWrapNeighbors( &layB );

	/* 4. CREATE AND ALLOCATE TILES */
	HitTile_float tileA, tileB, tileC;
	hit_tileSelectNoBoundary( &tileA, &A, hit_layMaxShape(layA));
	hit_tileSelectNoBoundary( &tileB, &B, hit_layMaxShape(layB) );
	hit_tileSelect( &tileC, &C,  hit_layShape(layC) );
	hit_tileAlloc( &tileA ); hit_tileAlloc( &tileB ); hit_tileAlloc( &tileC );
	//hit_tileClone( &copyTileA, &tileA ); hit_tileClone( &copyTileB, &tileB );

	/* 5. INITIALIZE MATRICES */
	//initMatrices( m, p, tileA, tileB, tileC, layA, layB );

	/* 6. INITIAL ALIGNMENT PHASE */
	HitCom commRow, commColumn;
	commRow = hit_comShiftDim( layA, 1, -hit_laySelfRanksDim(layA,0), &tileA, HIT_DOUBLE );
	commColumn = hit_comShiftDim( layB, 0, -hit_laySelfRanksDim(layB,1), &tileB, HIT_DOUBLE );

	hit_comDo( &commRow );
	hit_comDo( &commColumn );
	hit_comFree( commRow );
	hit_comFree( commColumn );

	/* 7. REPEATED COMM PATTERN */
	HitPattern shift = hit_pattern( HIT_PAT_UNORDERED );
	hit_comTagSet( TAG_A, TAG_B );
	hit_patternAdd( &shift, hit_comShiftDim( layA, 1, 1, &tileA, HIT_FLOAT ) );
	hit_patternAdd( &shift, hit_comShiftDim( layB, 0, 1, &tileB, HIT_FLOAT ) );

	/* 0. INIT CLOCKS */
//	hit_clockSynchronizeAll();
	//hit_clockStart( mainClock );

	// Init computation threads
	CALThread threads;
	CALThreadInit(threads, 2,  hit_tileDimCard(tileA,1), hit_tileDimCard(tileA,1) );
	printf("Dimensiones %d %d\n",hit_tileDimCard(tileA,0), hit_tileDimCard(tileA,1));	

	 if (rank<4) CAL_CntrlAttach(&comm, (HitTile*)&tileA);
	 if (rank<4) CAL_CntrlAttach(&comm, (HitTile*)&tileB);
	 if (rank<4) CAL_CntrlAttach(&comm, (HitTile*)&tileC);
	if (rank<4) CAL_CntrlSync(comm);
		 	/* 0. INIT CLOCKS */
	hit_clockSynchronizeAll();
	hit_clockStart( mainClock );
	/* 8. DO COMPUTATION */
	int loopIndex;
	int loopLimit = max( layA.numActives[0], layA.numActives[1] );
	
	printf("loopLimit %d %d %d\n", loopLimit, tileA.origAcumCard[0], tileA.origAcumCard[1] ); fflush(stdout);
	
	for (loopIndex = 0; loopIndex < loopLimit-1; loopIndex++) {
		/* 8.1. CLOCK TO MEASURE SECUENTIAL TIME OF THE PRODUCT */
		//hit_clockContinue( productClock );
		  if (rank>3) matrixBlockProduct( tileA, tileB, tileC );
		 if (rank<4) CAL_CntrlLaunch(comm, cannonCu, threads, tileA, tileB, tileC);
		 if (rank<4) CAL_CntrlSync(comm);
		hit_clockContinue( commClock );
		 if (rank<4) CAL_CntrlMoveFrom(&comm,(HitTile*)&tileA);
		 if (rank<4) CAL_CntrlMoveFrom(&comm,(HitTile*)&tileB);
		 if (rank<4) CAL_CntrlSync(comm);
		
		printf("loop \n"); fflush(stdout);
		
		
		/* 8.2. COMMUNICATE TILES */
		hit_patternDo( shift );
		if (rank<4) CAL_CntrlMoveTo(&comm,(HitTile*)&tileA);
		if (rank<4) CAL_CntrlMoveTo(&comm,(HitTile*)&tileB);
		if (rank<4) CAL_CntrlSync(comm);
		hit_clockStop( commClock );
	}
	/* 9. LAST ITERATION MULTIPLICATION: NO COMMUNICATION AFTER */
	//hit_clockContinue( productClock );
	if (rank>3) matrixBlockProduct( tileA, tileB, tileC );
	if (rank<4) CAL_CntrlLaunch(comm, cannonCu, threads, tileA, tileB, tileC);
	if (rank<4) CAL_CntrlSync(comm);
	hit_clockContinue( commClock );
	if (rank<4) CAL_CntrlDetach(&comm, (HitTile*)&tileC);
	if (rank<4) CAL_CntrlSync(comm);
	hit_clockStop( commClock );	

	/* 10. CLOCK RESULTS */
	hit_clockStop( mainClock );
	hit_clockReduce( layC, mainClock );
	hit_clockReduce( layC, commClock );
	hit_clockPrintMax( mainClock );
	hit_clockPrintMax( commClock );


    CAL_CntrlDestroy(&comm);
    



	/* 11. WRITE RESULT MATRIX */
#ifdef WRITE_RESULT
#ifdef WRITE_BIN
	hit_tileFileWrite( &tileC, "C.out", HIT_FILE_ARRAY );
#else
	hit_tileTextFileWrite( &tileC, "C.dtxt", HIT_FILE_ARRAY, HIT_FILE_FLOAT, 14, 4 );
#endif
#endif

	/* 12. FREE RESOURCES */
	hit_tileFree( tileA ); hit_tileFree( tileB ); hit_tileFree( tileC );
	hit_patternFree( &shift );
	hit_layFree( layA ); hit_layFree( layB ); hit_layFree( layC );
	hit_topFree( topo );
}

