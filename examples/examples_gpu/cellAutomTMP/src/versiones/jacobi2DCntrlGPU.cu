/*
* jacobi2D.c
* 	Hitmap example
* 	Stencil code: Jacobi 2D for the heat equation. Implemented as a 2D cellular automata. 
*
* v2.0
* (c) 2007-2015, Arturo Gonzalez-Escribano
*/

/*
 * <license>
 * 
 * Hitmap v1.2
 * 
 * This software is provided to enhance knowledge and encourage progress in the scientific
 * community. It should be used only for research and educational purposes. Any reproduction
 * or use for commercial purpose, public redistribution, in source or binary forms, with or 
 * without modifications, is NOT ALLOWED without the previous authorization of the copyright 
 * holder. The origin of this software must not be misrepresented; you must not claim that you
 * wrote the original software. If you use this software for any purpose (e.g. publication),
 * a reference to the software package and the authors must be included.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Copyright (c) 2007-2015, Trasgo Group, Universidad de Valladolid.
 * All rights reserved.
 * 
 * More information on http://trasgo.infor.uva.es/
 * 
 * </license>
*/

//#include <stdio.h>
//#include <stdlib.h>
//#include <hitmap.h>

#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "CAL.h"


hit_tileNewType( float );
hit_ktileNewType( float );

HitClock	mainClock;
HitClock	initClock;
HitClock	sequentialClock;


/* DEBUG: DUMP VARIABLE STRUCTURE INFORMATION */
void hit_dumpTileInternal(const void *var, const char* name, FILE *file) {
	int i;
	const HitTile *v = (const HitTile *)var;

	fprintf(file,"Dump tile %s, address %p\n",name,var);
	if (var == NULL) return;

	fprintf(file,"\tmemStatus: %d\n", v->memStatus);
	fprintf(file,"\tnumDims:   %d\n", hit_shapeDims(v->shape));
	fprintf(file,"\tbaseExtent:   %d\n", (unsigned int) v->baseExtent);
	for (i=0; i<hit_shapeDims(v->shape); i++) fprintf(file,"\tcard[%d]: %d\n", i, v->card[i]);
	fprintf(file,"\tacumCard:     %d\n", v->acumCard);
	for (i=0; i<hit_shapeDims(v->shape)+1; i++) fprintf(file,"\torigAcumCard[%d]: %d\n", i, v->origAcumCard[i]);
	for (i=0; i<hit_shapeDims(v->shape); i++) {
		fprintf(file,"\tqstride[%d]: %d\n", i, v->qstride[i]);
	}
	for (i=0; i<hit_shapeDims(v->shape); i++) {
		fprintf(file,"\tSig[%d]: %d, %d, %d\n", i, hit_shapeSig(v->shape,i).begin, hit_shapeSig(v->shape,i).end, hit_shapeSig(v->shape,i).stride);
	}
	fprintf(file,"\thierDepth:   %d\n", v->hierDepth);
	fprintf(file,"\tchildBegin: { ");
	for (i=0; i<HIT_MAXDIMS; i++) fprintf(file,"%d ", v->childBegin[i]);
	fprintf(file,"}\n");
	fprintf(file,"\tchildSize: { ");
	for (i=0; i<HIT_MAXDIMS; i++) fprintf(file,"%d ", v->childSize[i]);
	fprintf(file,"}\n");
	fprintf(file,"\tdata: %p\n", v->data);
	fprintf(file,"\tmemPtr: %p\n", v->memPtr);
	fprintf(file,"\tref: %p\n", (void*) v->ref);
	fprintf(file,"\n");
}


/* A. GPU Kernel charazterizations */
CAL_KERNEL_GPU_CHAR_STATIC(Copy, 2, full, low, low);
CAL_KERNEL_GPU_CHAR_STATIC(Update, 2, medium, medium, medium);

/* B.1 GPU Kernel implementation */
CAL_KERNEL(Copy, dGPU, KHitTile_float dst, KHitTile_float src){
	int row = threadId.y;
	int col = threadId.x;
	hit_tileElemAtNoStride(dst, 2, row, col) = hit_tileElemAtNoStride(src, 2, row, col);
}

/* B.2 GPU Kernel implementation */
CAL_KERNEL(Update, dGPU, KHitTile_float dst, KHitTile_float src){
	int row = threadId.y + 1;
	int col = threadId.x + 1;
	hit_tileElemAtNoStride(dst, 2, row, col) = ( 
			hit_tileElemAtNoStride(src, 2, row - 1, col    ) +
			hit_tileElemAtNoStride(src, 2, row + 1, col    ) +
			hit_tileElemAtNoStride(src, 2, row    , col - 1) +
			hit_tileElemAtNoStride(src, 2, row    , col + 1) ) / 4;
}

/* C. Defining kernel prototypes */
CAL_KERNEL_PROTO( Copy,                                               	// Name
                  1, dGPU,                                   // Implementations
                  2, OUT, HitTile_float, dst, IN, HitTile_float, src
                );

CAL_KERNEL_PROTO( Update,                                               // Name
                  1, dGPU,                                   // Implementations
                  2, OUT, HitTile_float, dst, IN, HitTile_float, src
                );
                
/* 9.1 Hit SHAPE EXPAND MANY DIMS IN EQUAL OFFSET */
HitShape hit_shapeExpand(HitShape shape,int dims,int offset){

	// Only works for Signature Shape.
	if(	hit_shapeType(shape) == HIT_CSR_SHAPE ) return HIT_SHAPE_NULL;

	HitShape res = shape;

	int dim;
	for(dim=0;dim<dims;dim++){
		hit_shapeSig(res,dim).begin-=offset;
		hit_shapeSig(res,dim).end+=offset;
	}
	return res;
}

/* 9.1 Hit SHAPE EXPAND A DIM IN A GIVEN DIRECTION */
HitShape hit_shapeDimExpand(HitShape shape,int dim, int position, int offset ){

	// Only works for Signature Shape.
	if(	hit_shapeType(shape) == HIT_CSR_SHAPE ) return HIT_SHAPE_NULL;

	HitShape res = shape;

	if ( position == HIT_SHAPE_BEGIN ) hit_shapeSig(res,dim).begin += offset;
	else hit_shapeSig(res,dim).end += offset;

	return res;
}

/* A. SEQUENTIAL: CELL UPDATE */
static inline void updateCell( float *myself, float up, float down, float left, float right ) {
	*myself = ( up + down + left + right ) / 4;
}


/* B. INITIALIZE MATRIX */
void initMatrix( HitTile_float tileMat ) {



	/* 0. OPTIONAL COMPILATION: READING THE INPUT MATRIX FROM A FILE */
//#ifdef READ_INPUT
//#ifdef READ_BIN
//	hit_tileFileRead( &tileMat, "Matrix.in", HIT_FILE_ARRAY );
//#else
//	hit_tileTextFileRead( &realTileA, "Matrix.in.dtxt", HIT_FILE_ARRAY, HIT_FILE_DOUBLE, 14, 4 );
//#endif
//#else
	/* 1. INIT Mat = 0 */
	float zero=0; 
	hit_tileFill( &tileMat, &zero );

	/* 2. INIT BORDERS  UP(i)=1, DOWN(i)=2, LEFT(i)=3, RIGHT(i)=4 */
	HitTile root = *hit_tileRoot( &tileMat );
	int i,j;

	/* 2.1. FIRST COLUMN IS MINE */
	if ( hit_sigIn( hit_tileDimSig( tileMat, 1 ), hit_tileDimBegin( root, 1 ) ) )
		hit_tileForDimDomain( tileMat, 0, i )
			hit_tileElemAt( tileMat, 2, i, 0 ) = 3;

	/* 2.2. LAST COLUMN IS MINE */
	if ( hit_sigIn( hit_tileDimSig( tileMat, 1 ), hit_tileDimEnd( root, 1 ) ) )
		hit_tileForDimDomain( tileMat, 0, i )
			hit_tileElemAt( tileMat, 2, i, hit_tileDimCard( tileMat, 1 )-1 ) = 4;

	/* 2.3. FIRST ROW IS MINE */
	if ( hit_sigIn( hit_tileDimSig( tileMat, 0 ), hit_tileDimBegin( root, 0 ) ) )
		hit_tileForDimDomain( tileMat, 1, j )
			hit_tileElemAt( tileMat, 2, 0, j ) = 1;
		
	/* 2.4. LAST ROW IS MINE */
	if ( hit_sigIn( hit_tileDimSig( tileMat, 0 ), hit_tileDimEnd( root, 0 ) ) )
		hit_tileForDimDomain( tileMat, 1, j )
			hit_tileElemAt( tileMat, 2, hit_tileDimCard( tileMat, 0 )-1, j ) = 2;
//#endif

//#ifdef WRITE_INPUT
	/* 4. WRITE MAT ON A FILE */
//#ifdef WRITE_BIN
	hit_tileFileWrite( &tileMat, "Matrix.out", HIT_FILE_ARRAY );
//#else
//#ifdef WRITE_TILE_FILES
//	char name[8];
//	sprintf(name, "Matrix.%d", hit_Rank );
//	hit_tileTextFileWrite( &timeMat, name, HIT_FILE_TILE, HIT_FILE_DOUBLE, 14, 4 );
//#endif
//	hit_tileTextFileWrite( &tileMat, "Matrix.out.dtxt", HIT_FILE_ARRAY, HIT_FILE_DOUBLE, 14, 4 );
//#endif
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/* C. MAIN: CELLULAR AUTOMATA */
int main(int argc, char *argv[]) {

        // 1. Init Controllers library
        //CAL_CntrlInit(2);

        
        omp_set_nested(1);
        omp_set_num_threads(3);
        #pragma omp parallel 
        {
        #pragma omp single 
         {


	printf("HOLA 1: main\n");

	hit_comInit( &argc, &argv );

	int rows, columns, numIter;

	if ( argc != 4 ) {
		fprintf(stderr, "\nUsage: %s <numRows> <numColumns> <numIterations>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
//	rows = atoi( argv[1] );
//	columns = atoi( argv[2] );
//	numIter = atoi( argv[3] );	

	rows = atoi( argv[1] );
	columns = atoi( argv[1] );
	numIter = atoi( argv[2] );	
	int GPU = atoi( argv[3] );	
	
	
	/* 0. INIT CLOCKS */
	hit_clockSynchronizeAll();
	hit_clockStart( mainClock );
	hit_clockStart( initClock );
	hit_clockReset( sequentialClock );

    /* 1. CREATE VIRTUAL TOPOLOGY */
    HitTopology topo = hit_topology( plug_topArray2DComplete );

	/* 2. DECLARE FULL MATRIX WITHOUT MEMORY */
	HitTile_float matrix;
	hit_tileDomain( &matrix, float, 2, rows, columns );

    /* 3. COMPUTE PARTITION */
	HitShape parallelShape = hit_tileShape( matrix );
	parallelShape = hit_shapeExpand( parallelShape, 2, -1 );
	HitLayout matLayout = hit_layout( plug_layBlocks, topo, parallelShape );

	/* 4. ACTIVE PROCESSES */
	if ( hit_layImActive( matLayout ) ) {

		/* 4.1. CREATE AND ALLOCATE LOCAL TILES */
		/* 4.1.1. LOCAL TILE WITH SPACE FOR FOREIGN DATA */
		HitTile_float tileMat, tileCopy;
		HitShape expandedShape = hit_shapeDimExpand( hit_layShape(matLayout), 0, HIT_SHAPE_BEGIN, -1 );
		expandedShape = hit_shapeDimExpand( expandedShape, 0, HIT_SHAPE_END, 1 );
		expandedShape = hit_shapeDimExpand( expandedShape, 1, HIT_SHAPE_BEGIN, -1 );
		expandedShape = hit_shapeDimExpand( expandedShape, 1, HIT_SHAPE_END, 1 );
		hit_tileSelect( &tileMat, &matrix, expandedShape );
		hit_tileAlloc( &tileMat );


		/* 4.1.2. COPY OF THE TILE, TO AVOID SEQUENTIAL SEMANTICS ON UPDATES */
		hit_tileSelect( &tileCopy, &tileMat, HIT_SHAPE_WHOLE );
		hit_tileAlloc( &tileCopy );

#ifdef DEBUG
		hit_dumpTileFile( tileMat, "Tile", "Matrix tile" );
#endif

		/* 4.2. COMMUNICATION PATTERN */
		hit_comTagSet( TAG_UP, TAG_DOWN, TAG_LEFT, TAG_RIGHT );
		HitPattern neighSync = hit_pattern( HIT_PAT_UNORDERED );
		hit_patternAdd( &neighSync, 
				hit_comSendRecvSelectTag( matLayout, 
							hit_layNeighbor( matLayout, 0, -1 ) , &tileMat, 
							hit_shape( 2, hit_sigIndex(1), hit_sig( 1, hit_tileDimCard(tileMat,1)-2, 1) ),
							HIT_COM_TILECOORDS, 
							hit_layNeighbor( matLayout, 0, +1 ) , &tileMat, 
							hit_shape( 2, hit_sigIndex( hit_tileDimCard(tileMat,0)-1 ), hit_sig( 1, hit_tileDimCard(tileMat,1)-2, 1) ),
							HIT_COM_TILECOORDS, 
							HIT_FLOAT, TAG_UP )
							);
		hit_patternAdd( &neighSync, 
				hit_comSendRecvSelectTag( matLayout, 
							hit_layNeighbor( matLayout, 0, +1 ) , &tileMat, 
							hit_shape( 2, hit_sigIndex( hit_tileDimCard(tileMat,0)-2 ), hit_sig( 1, hit_tileDimCard(tileMat,1)-2, 1) ),
							HIT_COM_TILECOORDS, 
							hit_layNeighbor( matLayout, 0, -1 ) , &tileMat, 
							hit_shape( 2, hit_sigIndex(0), hit_sig( 1, hit_tileDimCard(tileMat,1)-2, 1) ),
							HIT_COM_TILECOORDS, 
							HIT_FLOAT, TAG_DOWN )
							);
		hit_patternAdd( &neighSync, 
				hit_comSendRecvSelectTag( matLayout, 
							hit_layNeighbor( matLayout, 1, -1 ) , &tileMat, 
							hit_shape( 2, hit_sig( 1, hit_tileDimCard(tileMat,0)-2, 1), hit_sigIndex(1) ),
							HIT_COM_TILECOORDS, 
							hit_layNeighbor( matLayout, 1, +1 ) , &tileMat, 
							hit_shape( 2, hit_sig( 1, hit_tileDimCard(tileMat,0)-2, 1), hit_sigIndex( hit_tileDimCard(tileMat,1)-1 ) ),
							HIT_COM_TILECOORDS, 
							HIT_FLOAT, TAG_LEFT )
							);
		hit_patternAdd( &neighSync, 
				hit_comSendRecvSelectTag( matLayout, 
							hit_layNeighbor( matLayout, 1, +1 ) , &tileMat, 
							hit_shape( 2, hit_sig( 1, hit_tileDimCard(tileMat,0)-2, 1), hit_sigIndex( hit_tileDimCard(tileMat,1)-2 ) ),
							HIT_COM_TILECOORDS, 
							hit_layNeighbor( matLayout, 1, -1 ) , &tileMat, 
							hit_shape( 2, hit_sig( 1, hit_tileDimCard(tileMat,0)-2, 1), hit_sigIndex(0) ),
							HIT_COM_TILECOORDS, 
							HIT_FLOAT, TAG_RIGHT )
							);

		/* 4.3. INITIALIZE MATRIX (IN PARALLEL) */

		initMatrix( tileCopy );											//senmao

        // 4. Init computation threads
		CALThread threads;												//yuri
		CALThreadInit(threads, 2, rows, columns);
		CALThread update;
		CALThreadInit(update, 2, rows-2, columns-2)


		//Declaración y creación de controlador
		CALCntrl cntrl_A;												//yuri
		CAL_CntrlCreate(&cntrl_A, CAL_CNTRL_GPU, GPU);					//yuri
		


//		cudaStream_t stream_A;												//yuri
//		cudaStreamCreate ( &stream_A );										//yuri

		
										HitTile_float mat;
										HitTile_float copy;
	
										HitTile_float mat_A;	//Yuri
										HitTile_float copy_A;	//Yuri
	
										int SIZE = 10;
	
										hit_tileDomain ( &mat, float, 2, SIZE, SIZE );
										hit_tileDomain ( &copy, float, 2, SIZE, SIZE );
										hit_tileAlloc( &mat );
	
	
										hit_tileDomain ( &mat_A, float, 2, SIZE, SIZE );	//Yuri
										hit_tileDomain ( &copy_A, float, 2, SIZE, SIZE );	//Yuri
										hit_tileAlloc( &mat_A );
	
										CALThread threads2;
										CALThreadInit(threads2, 2, SIZE, SIZE);
										CALThread update2;
										CALThreadInit(update2, 2, SIZE-2, SIZE-2)
										CALCntrl comm2;
										CAL_CntrlCreate(&comm2, CAL_CNTRL_GPU, GPU);
										CAL_CntrlAttach(&comm2, (HitTile*)&mat);
										CAL_CntrlInternal(&comm2, (HitTile*)&copy);
										CAL_CntrlLaunch(comm2, Copy, threads, copy, mat);			
										CAL_CntrlDestroy(&comm2);										//yuri
		//hit_tileAlloc( &matrix );
		//hit_dumpTile(matrix);


		//CAL_CntrlLaunch(cntrl_A, Copy, threads, tileCopy, tileMat);			

		//inicializo la copia y e patron de copia  
		//attacheo los dos		***

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/* 4.4. COMPUTATION LOOP */
		int i,j;
		int loopIndex;
		for (loopIndex = 0; loopIndex < /*numIter-1*/1; loopIndex++) {
			
			//Ejecutar patron de comunicacion 2
			//pasar a la gpu los 4 halos del copy
			
			
			printf("Iteración Inicial:      %d\n", loopIndex);

			CAL_CntrlAttach(&cntrl_A, (HitTile*)&tileMat);					//yuri
			CAL_CntrlAttach(&cntrl_A, (HitTile*)&tileCopy);					//yuri

			
			//CAL_CntrlLaunch(cntrl_A, Copy, threads, tileCopy, tileMat);			
			CAL_CntrlLaunch(cntrl_A, Update, update, tileMat, tileCopy);

			//cudaDeviceSynchronize();
			
			CAL_CntrlDetach(&cntrl_A, (HitTile*)&tileMat);
			cudaDeviceSynchronize();
			
			printf("Iteración Final:      %d\n\n", loopIndex);
			
			
			// CLOCK: Measure secuential time 
//			hit_clockContinue( sequentialClock );

			/* 4.4.1. UPDATE TILE COPY */
//			hit_tileUpdateFromAncestor( &tileCopy );

			/* 4.4.2. SEQUENTIALIZED LOOP */
//			for ( i=1; i<hit_tileDimCard( tileMat, 0 )-1; i++ )
//				for ( j=1; j<hit_tileDimCard( tileMat, 1 )-1; j++ )
//					updateCell( 
//						& hit_tileElemAt( tileMat, 2, i, j ),
//						hit_tileElemAt( tileCopy, 2, i-1, j ),
//						hit_tileElemAt( tileCopy, 2, i+1, j ),
//						hit_tileElemAt( tileCopy, 2, i, j-1 ),
//						hit_tileElemAt( tileCopy, 2, i, j+1 )
//						);
//			hit_clockStop( sequentialClock );

			/* 4.4.3. COMMUNICATE */
			hit_patternDo( neighSync );
		}//loop (loopIndex)
		CAL_CntrlDestroy(&cntrl_A);										//yuri
		/* 4.5. LAST ITERATION UPDATE: NO COMMUNICATION AFTER */
		hit_clockContinue( sequentialClock );
		hit_tileUpdateFromAncestor( &tileCopy );
		for ( i=1; i<hit_tileDimCard( tileMat, 0 )-1; i++ )
			for ( j=1; j<hit_tileDimCard( tileMat, 1 )-1; j++ )
				updateCell( 
					& hit_tileElemAt( tileMat, 2, i, j ),
					hit_tileElemAt( tileCopy, 2, i-1, j ),
					hit_tileElemAt( tileCopy, 2, i+1, j ),
					hit_tileElemAt( tileCopy, 2, i, j-1 ),
					hit_tileElemAt( tileCopy, 2, i, j+1 )
					);
		hit_clockStop( sequentialClock );

		/* 4.6. CLOCK RESULTS */
		hit_clockStop( mainClock );
		hit_clockReduce( matLayout, mainClock );
		hit_clockReduce( matLayout, sequentialClock );
		hit_clockPrintMax( mainClock );
		hit_clockPrintMax( sequentialClock );

		/* 4.7. WRITE RESULT MATRIX */
#ifdef WRITE_RESULT
		HitTile root = *hit_tileRoot( &tileMat );
		HitTile_float outputTile;
		HitShape outputShape = hit_tileShape( tileMat );
		/* FIRST ROW IS NOT MINE */
		if ( ! hit_sigIn( hit_shapeSig( outputShape, 0 ), hit_tileDimBegin( root, 0 ) ) )
			outputShape	= hit_shapeDimExpand( outputShape, 0, HIT_SHAPE_BEGIN, +1 );
		/* LAST ROW IS NOT MINE */
		if ( ! hit_sigIn( hit_shapeSig( outputShape, 0 ), hit_tileDimEnd( root, 0 ) ) )
			outputShape = hit_shapeDimExpand( outputShape, 0, HIT_SHAPE_END, -1 );
		/* FIRST COLUMN IS NOT MINE */
		if ( ! hit_sigIn( hit_shapeSig( outputShape, 1 ), hit_tileDimBegin( root, 1 ) ) )
			outputShape = hit_shapeDimExpand( outputShape, 1, HIT_SHAPE_BEGIN, +1 );
		/* LAST COLUMN IS NOT MINE */
		if ( ! hit_sigIn( hit_shapeSig( outputShape, 1 ), hit_tileDimEnd( root, 1 ) ) )
			outputShape = hit_shapeDimExpand( outputShape, 1, HIT_SHAPE_END, -1 );
		hit_tileSelectArrayCoords( &outputTile, &tileMat, outputShape );
#ifdef WRITE_BIN
		hit_tileFileWrite( &outputTile, "Result.out", HIT_FILE_ARRAY );
#else
		hit_tileTextFileWrite( &outputTile, "Result.out.dtxt", HIT_FILE_ARRAY, HIT_FILE_DOUBLE, 14, 4 );
#endif
#endif
		/* 4.8. FREE RESOURCES */
		hit_tileFree( tileMat );
		hit_tileFree( tileCopy );
		hit_patternFree( &neighSync );
		
			
	
	}
	/* 5. INACTIVE PROCESSES: ONLY COLLECTIVE CLOCK OPERATIONS */
	else {
		hit_clockStop( initClock );
		hit_clockStop( mainClock );
		hit_clockReduce( matLayout, mainClock );
		hit_clockReduce( matLayout, sequentialClock );
		hit_clockPrintMax( mainClock );
		hit_clockPrintMax( sequentialClock );
		
	}

	/* 6. FREE OTHER RESOURCES */

	hit_layFree( matLayout );
	hit_topFree( topo );

		//hit_comFinalize();
		// 13. Finish the controller library
        CAL_CntrlFinish();
        
        

	return 0;
}
