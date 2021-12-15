#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "CAL.h"


/*******************************************/


	/*Tipo tiles*/
	hit_tileNewType( float );
	hit_ktileNewType( float );	

	/*Relojes*/


	//
	/*Caracterización de kernels*/
	//
	CAL_KERNEL_GPU_CHAR_STATIC(UpdateEdgeU, 2, medium, medium, medium);
	CAL_KERNEL_GPU_CHAR_STATIC(UpdateEdgeD, 2, medium, medium, medium);
	CAL_KERNEL_GPU_CHAR_STATIC(UpdateEdgeL, 2, medium, medium, medium);
	CAL_KERNEL_GPU_CHAR_STATIC(UpdateEdgeR, 2, medium, medium, medium);
	CAL_KERNEL_GPU_CHAR_STATIC(UpdateThick, 2, medium, medium, medium);
	//
	CAL_KERNEL_GPU_CHAR_STATIC(CopyEdgeU, 2, medium, medium, medium);
	CAL_KERNEL_GPU_CHAR_STATIC(CopyEdgeD, 2, medium, medium, medium);
	CAL_KERNEL_GPU_CHAR_STATIC(CopyEdgeL, 2, medium, medium, medium);
	CAL_KERNEL_GPU_CHAR_STATIC(CopyEdgeR, 2, medium, medium, medium);	
	CAL_KERNEL_GPU_CHAR_STATIC(CopyThick, 2, medium, medium, medium);	
	

	//
	/*Implementación de kernels*/
	//
	CAL_KERNEL(UpdateEdgeU, dGPU, KHitTile_float dst, KHitTile_float edgeU, KHitTile_float src){
		int row = threadId.y + 1;
		int col = threadId.x + 1;
	
			hit_tileElemAtNoStride( dst, 2, row , col ) =  (
				hit_tileElemAtNoStride(edgeU, 2, 0, col    ) +
				hit_tileElemAtNoStride(src, 2, row + 1, col    ) +
				hit_tileElemAtNoStride(src, 2, row    , col - 1) +
				hit_tileElemAtNoStride(src, 2, row    , col + 1) ) / 4  ;
				
				
	}//UpdateEdgeU
	
	CAL_KERNEL(UpdateEdgeD, dGPU, KHitTile_float dst, KHitTile_float edgeD, KHitTile_float src){
		int row = threadId.y + src.card[0] - 2;
		int col = threadId.x + 1;
		
			hit_tileElemAtNoStride( dst, 2, row, col ) =  (
				hit_tileElemAtNoStride(edgeD, 2, 0, col    ) +
				hit_tileElemAtNoStride(src, 2, row - 1, col    ) +
				hit_tileElemAtNoStride(src, 2, row    , col - 1) +
				hit_tileElemAtNoStride(src, 2, row    , col + 1) ) / 4  ;
				
				
	}//UpdateEdgeD
	
	CAL_KERNEL(UpdateEdgeL, dGPU, KHitTile_float dst, KHitTile_float edgeL, KHitTile_float src){
		int row = threadId.y + 1 ;
		int col = threadId.x + 1;
		
			hit_tileElemAtNoStride( dst, 2, row, col ) =  (
				hit_tileElemAtNoStride(edgeL, 2, 0, row    ) +
				hit_tileElemAtNoStride(src, 2, row + 1, col    ) +
				hit_tileElemAtNoStride(src, 2, row - 1 , col) +
				hit_tileElemAtNoStride(src, 2, row    , col + 1) ) / 4  ;
				
	}//UpdateEdgeL	

	CAL_KERNEL(UpdateEdgeR, dGPU, KHitTile_float dst, KHitTile_float edgeR, KHitTile_float src){
		int row = threadId.y + 1;
		int col = threadId.x + src.card[1] - 2;
		
			hit_tileElemAtNoStride( dst, 2, row, col ) =  (
				hit_tileElemAtNoStride(edgeR, 2, 0, row    ) +
				hit_tileElemAtNoStride(src, 2, row + 1, col    ) +
				hit_tileElemAtNoStride(src, 2, row - 1   , col) +
				hit_tileElemAtNoStride(src, 2, row    , col - 1) ) / 4  ;
			
	}//UpdateEdgeR	

	CAL_KERNEL(UpdateThick, dGPU, KHitTile_float dst, KHitTile_float src){
		int row = threadId.y + 2;
		int col = threadId.x + 2;
		
			hit_tileElemAtNoStride( dst, 2, row, col ) =  (
				hit_tileElemAtNoStride(src, 2, row + 1, col    ) +
				hit_tileElemAtNoStride(src, 2, row - 1, col    ) +
				hit_tileElemAtNoStride(src, 2, row    , col - 1) +
				hit_tileElemAtNoStride(src, 2, row    , col + 1) ) / 4  ;
	}//UpdateEdgeR	

	
	CAL_KERNEL(CopyEdgeU, dGPU, KHitTile_float edgeU, KHitTile_float src){
		int row = threadId.y + 1;
		int col = threadId.x;
		hit_tileElemAtNoStride(edgeU, 2, 0, col) = hit_tileElemAtNoStride(src, 2, row, col);
		
						
		
	}//CopyEdgeD

	CAL_KERNEL(CopyEdgeD, dGPU, KHitTile_float edgeD, KHitTile_float src){
		int row = threadId.y + src.card[0] - 2;
		int col = threadId.x;
		hit_tileElemAtNoStride(edgeD, 2, 0, col) = hit_tileElemAtNoStride(src, 2, row, col);
		
	}//CopyEdgeD

	CAL_KERNEL(CopyEdgeL, dGPU, KHitTile_float edgeL, KHitTile_float src){
		int row = threadId.y;
		int col = threadId.x + 1;
		hit_tileElemAtNoStride(edgeL, 2, 0, row) = hit_tileElemAtNoStride(src, 2, row, col);

	}//CopyEdgeD

	CAL_KERNEL(CopyEdgeR, dGPU, KHitTile_float edgeR, KHitTile_float src){
		int row = threadId.y;
		int col = threadId.x + src.card[1] - 2;
		hit_tileElemAtNoStride(edgeR, 2, 0, row) = hit_tileElemAtNoStride(src, 2, row, col);
	
	}//CopyEdgeD

	CAL_KERNEL(CopyThick, dGPU, KHitTile_float dst, KHitTile_float src){
		int row = threadId.y + 1;
		int col = threadId.x + 1;
		hit_tileElemAtNoStride(dst, 2, row, col) = hit_tileElemAtNoStride(src, 2, row, col);		
	}//CopyThick


	//
	/*Declaración de prototipos*/
	//
	CAL_KERNEL_PROTO( UpdateEdgeU,                                               // Name
                  1, dGPU,        						                         // Implementations
                  3, OUT, HitTile_float, dst, IN, HitTile_float, edgeU, IN, HitTile_float, src 
               );

	CAL_KERNEL_PROTO( UpdateEdgeD,                                               // Name
                  1, dGPU,        						                         // Implementations
                  3, OUT, HitTile_float, dst, IN, HitTile_float, edgeD, IN, HitTile_float, src 
               );

	CAL_KERNEL_PROTO( UpdateEdgeL,                                               // Name
                  1, dGPU,        						                         // Implementations
                  3, OUT, HitTile_float, dst, IN, HitTile_float, edgeL, IN, HitTile_float, src 
               );

	CAL_KERNEL_PROTO( UpdateEdgeR,                                               // Name
                  1, dGPU,        						                         // Implementations
                  3, OUT, HitTile_float, dst, IN, HitTile_float, edgeR, IN, HitTile_float, src 
               );

	CAL_KERNEL_PROTO( UpdateThick,                                               // Name
                  1, dGPU,        						                         // Implementations
                  2, OUT, HitTile_float, dst, IN, HitTile_float, src 
               );

	CAL_KERNEL_PROTO( CopyEdgeU,                                               	// Name
                  1, dGPU,                                   					// Implementations
                  2, OUT, HitTile_float, edgeU, IN, HitTile_float, src
                );

	CAL_KERNEL_PROTO( CopyEdgeD,                                               	// Name
                  1, dGPU,                                   					// Implementations
                  2, OUT, HitTile_float, edgeD, IN, HitTile_float, src
                );

	CAL_KERNEL_PROTO( CopyEdgeL,                                               	// Name
                  1, dGPU,                                   					// Implementations
                  2, OUT, HitTile_float, edgeL, IN, HitTile_float, src
                );

	CAL_KERNEL_PROTO( CopyEdgeR,                                               	// Name
                  1, dGPU,                                   					// Implementations
                  2, OUT, HitTile_float, edgeR, IN, HitTile_float, src
                );

	CAL_KERNEL_PROTO( CopyThick,                                               	// Name
                  1, dGPU,                                   			// Implementations
                  2, OUT, HitTile_float, dst, IN, HitTile_float, src
                );

                


	/*hit_shapeExpand. Debería de no tener que estar aquí*/
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
	}//hit_shapeExpand


	/*hit_shapeDimExpand. Debería de no tener que estar aquí*/ 
	HitShape hit_shapeDimExpand(HitShape shape,int dim, int position, int offset ){

		// Only works for Signature Shape.
		if(	hit_shapeType(shape) == HIT_CSR_SHAPE ) return HIT_SHAPE_NULL;

		HitShape res = shape;

		if ( position == HIT_SHAPE_BEGIN ) hit_shapeSig(res,dim).begin += offset;
		else hit_shapeSig(res,dim).end += offset;

		return res;
	}//hit_shapeDimExpand

	/* Dump de los tiles (los imprime) */
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
	}//hit_dumpTileInternal


	
	/*Inicialización de matrices*/
	void initMatrix( HitTile_float tileMat ) {

			/* 0. OPTIONAL COMPILATION: READING THE INPUT MATRIX FROM A FILE */
		#ifdef READ_INPUT
		#ifdef READ_BIN
			hit_tileFileRead( &tileMat, "Matrix.in", HIT_FILE_ARRAY );
		#else
			hit_tileTextFileRead( &realTileA, "Matrix.in.dtxt", HIT_FILE_ARRAY, HIT_FILE_FLOAT, 14, 4 );
		#endif
		#else
		/* 1. INIT Mat = 0 */
		double zero=0; 
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
		#endif

		#ifdef WRITE_INPUT
		/* 4. WRITE MAT ON A FILE */
		#ifdef WRITE_BIN
			hit_tileFileWrite( &tileMat, "Matrix.out", HIT_FILE_ARRAY );
		#else
		#ifdef WRITE_TILE_FILES
			char name[8];
			sprintf(name, "Matrix.%d", hit_Rank );
			hit_tileTextFileWrite( &timeMat, name, HIT_FILE_TILE, HIT_FILE_FLOAT, 14, 4 );
		#endif
			hit_tileTextFileWrite( &tileMat, "Matrix.out.dtxt", HIT_FILE_ARRAY, HIT_FILE_FLOAT, 14, 4 );
		#endif		
		#endif
	}//initMatrix
	
	
		
/* C. MAIN: CELLULAR AUTOMATA */
int main(int argc, char *argv[]) {
	
		hit_comInit( &argc, &argv );
        
        /*Hilos OpenMP*/
        omp_set_nested(1);
        omp_set_num_threads(3);
        #pragma omp parallel 
        {
        #pragma omp single 
         {

			int rows, columns, numIter;

			if ( argc != 4 ) {
				fprintf(stderr, "\nUsage: %s <numRows> <numColumns> <numIterations>\n", argv[0]);
				exit(EXIT_FAILURE);
			}		
		
		/*Dimensiones/cardinalidades del Stencil*/
		rows = atoi( argv[1] );
		columns = atoi( argv[1] );
		numIter = atoi( argv[2] );	
		int GPU = atoi( argv[3] );	

		/*Declaración de relojes*/
		Timer tTotal;
		Timer tParcial;
		TimerCreate(tTotal);
		TimerCreate(tParcial);


		CALThread thUpdateEdgesUD, thCopyEdgesUD; 
		CALThread thUpdateEdgesLR, thCopyEdgesLR;
		CALThread thUpdateThick, thCopyThick;
		//
		CALThreadInit(thUpdateEdgesUD, 2, 1, columns - 2); CALThreadInit(thCopyEdgesUD, 2, 1, columns); 
		CALThreadInit(thUpdateEdgesLR, 2, rows - 2, 1); CALThreadInit(thCopyEdgesLR, 2, rows, 1);							
		CALThreadInit(thUpdateThick, 2, rows - 2, columns - 2); CALThreadInit(thCopyThick, 2, rows - 2, columns - 2);
		//
		HitTile_float  tileHaloU, tileHaloD, tileHaloL, tileHaloR;
		HitShape uShape; 

		/*Creamos topología*/
		HitTopology topo = hit_topology( plug_topArray2DComplete );
		
		/*Creación de tiles*/
		HitTile_float matrix;
		hit_tileDomain( &matrix, float, 2, rows, columns );

		/*Creamos el shape y el layout*/
		HitShape parallelShape = hit_tileShape( matrix );
		parallelShape = hit_shapeExpand( parallelShape, 2, -1 );
		HitLayout matLayout = hit_layout( plug_layBlocks, topo, parallelShape );


			/*Si soy proceso activo...*/
			if ( hit_layImActive( matLayout ) ) {
		
				/*Datos locales con extensiones*/
				HitTile_float tileMat, tileCopy;
				HitShape expandedShape = hit_shapeDimExpand( hit_layShape(matLayout), 0, HIT_SHAPE_BEGIN, -1 );
				expandedShape = hit_shapeDimExpand( expandedShape, 0, HIT_SHAPE_END, 1 );
				expandedShape = hit_shapeDimExpand( expandedShape, 1, HIT_SHAPE_BEGIN, -1 );
				expandedShape = hit_shapeDimExpand( expandedShape, 1, HIT_SHAPE_END, 1 );
				hit_tileSelect( &tileMat, &matrix, expandedShape );
				hit_tileAlloc( &tileMat );
				//
				hit_tileSelect( &tileCopy, &tileMat, HIT_SHAPE_WHOLE );
				hit_tileAlloc( &tileCopy );
		
				/*Inicialización de matrices*/
				initMatrix( tileMat );
				initMatrix( tileCopy );

				/*Patrón de comunicación*/
				HitPattern pat = hit_patternLayRedistribute(matLayout, matLayout, &tileCopy, &tileMat, HIT_FLOAT);


				/*Declaración y Selección de halos (hacemos duplicado de halos)*/
				//				
				hit_tileSelect( &tileHaloU, &tileMat, 
						hit_shape2( hit_sig(hit_tileDimSig(tileMat,0).begin, hit_tileDimSig(tileMat,0).begin, 
						hit_tileDimSig(tileMat,0).stride), hit_tileDimSig(tileMat,1) ) );				
				//	
				hit_tileSelect( &tileHaloD, &tileMat, 
						hit_shape2( hit_sig(hit_tileDimSig(tileMat,0).end, hit_tileDimSig(tileMat,0).end, 
						hit_tileDimSig(tileMat,0).stride) , hit_tileDimSig(tileMat,1) ) );								
				//
				hit_tileSelect( &tileHaloL, &tileMat, 
						hit_shape2( hit_tileDimSig(tileMat,0),hit_sig(hit_tileDimSig(tileMat,1).begin, 
							hit_tileDimSig(tileMat,1).begin, hit_tileDimSig(tileMat,1).stride) ) );					
				//
				hit_tileSelect( &tileHaloR, &tileMat, 
						hit_shape2( hit_tileDimSig(tileMat,0), hit_sig(hit_tileDimSig(tileMat,1).end, 
							hit_tileDimSig(tileMat,1).end, hit_tileDimSig(tileMat,1).stride) ) );					
				//
				hit_tileAlloc( &tileHaloU ); hit_tileUpdateFromAncestor( &tileHaloU );
				hit_tileAlloc( &tileHaloD ); hit_tileUpdateFromAncestor( &tileHaloD );
				hit_tileAlloc( &tileHaloL ); hit_tileUpdateFromAncestor( &tileHaloL );
				hit_tileAlloc( &tileHaloR ); hit_tileUpdateFromAncestor( &tileHaloR );
				
				/*Declaración y creación de streams*/
				cudaStream_t stream_U, stream_D, stream_L, stream_R, stream_Mat, stream_Co;				
				cudaStreamCreate ( &stream_U );		cudaStreamCreate ( &stream_D );
				cudaStreamCreate ( &stream_L );		cudaStreamCreate ( &stream_R );
				cudaStreamCreate ( &stream_Mat );	cudaStreamCreate ( &stream_Co );										
#ifdef DEBUG
				/*Imprimimos resultados parciales*/
				for (int i =0;i < rows; i++)
				{for (int j =0;j < columns; j++){
						printf("%d ", (int) hit_tileElemAt2(tileMat,i,j));;
						}printf("\n");}					

				printf("\n\nU    D  L  R\n");
				printf("-    -  -  -\n");
				for (int i =0;i < rows; i++)
				{for (int j =0;j < 1; j++){
						printf("%d    %d  %d  %d",	(int) hit_tileElemAt1(tileHaloU,i+j), 
													(int) hit_tileElemAt1(tileHaloD,i+j), 
													(int) hit_tileElemAt2(tileHaloL,i,j), 
													(int) hit_tileElemAt2(tileHaloR,i,j));
						}printf("\n");}	
#endif
						
				/*Declaración y creación de controlador*/
				CALCntrl cntrl_GPU;
				CAL_CntrlCreate(&cntrl_GPU, CAL_CNTRL_GPU, GPU);
TimerStart(tTotal);	//ttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
TimerStart(tParcial);//ppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp
#define ASYN				
				/*Attach variables a la GPU*/
#ifdef SYN
				printf("\n**Version Sincrona**\n");
				CAL_CntrlAttach(&cntrl_GPU, (HitTile*)&tileMat); CAL_CntrlAttach(&cntrl_GPU, (HitTile*)&tileCopy);
				CAL_CntrlAttach(&cntrl_GPU,(HitTile*)&tileHaloU); CAL_CntrlAttach(&cntrl_GPU,(HitTile*)&tileHaloD);
				CAL_CntrlAttach(&cntrl_GPU,(HitTile*)&tileHaloL); CAL_CntrlAttach(&cntrl_GPU,(HitTile*)&tileHaloR);
		
#else				
				printf("\n**Version A_Sincrona**\n");
  				CAL_CntrlAttachAsyn(&cntrl_GPU, (HitTile*)&tileMat, stream_Mat); CAL_CntrlAttachAsyn(&cntrl_GPU, (HitTile*)&tileCopy, stream_Co);
				CAL_CntrlAttachAsyn(&cntrl_GPU,(HitTile*)&tileHaloU, stream_U);  CAL_CntrlAttachAsyn(&cntrl_GPU,(HitTile*)&tileHaloD, stream_D);
				CAL_CntrlAttachAsyn(&cntrl_GPU,(HitTile*)&tileHaloL, stream_L);  CAL_CntrlAttachAsyn(&cntrl_GPU,(HitTile*)&tileHaloR, stream_R);
#endif
			
				CAL_CntrlDestroy(&cntrl_GPU);							//*****************
TimerStop(tParcial);//ppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp				

									
				hit_patternDo( pat );
	
				CAL_CntrlCreate(&cntrl_GPU, CAL_CNTRL_GPU, GPU);		//*****************								
				/*Iniciar cómputo bordes, declaración de hilos*/
	
						
				//Borde U
TimerContinue(tParcial);//ppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp					
#ifdef SYN
				CAL_CntrlLaunch(cntrl_GPU, UpdateEdgeU, thUpdateEdgesUD, tileCopy, tileHaloU, tileMat); 
				CAL_CntrlLaunch(cntrl_GPU, CopyEdgeU, thCopyEdgesUD, tileHaloU, tileCopy);
#else
				CAL_CntrlLaunchCKernel(cntrl_GPU, UpdateEdgeU, thUpdateEdgesUD, stream_U, tileCopy, tileHaloU, tileMat); 
				CAL_CntrlLaunchCKernel(cntrl_GPU, CopyEdgeU, thCopyEdgesUD, stream_U, tileHaloU, tileCopy);
#endif

				//Borde D
#ifdef SYN				
				CAL_CntrlLaunch(cntrl_GPU, UpdateEdgeD, thUpdateEdgesUD, tileCopy, tileHaloD, tileMat);
				CAL_CntrlLaunch(cntrl_GPU, CopyEdgeD, thCopyEdgesUD, tileHaloD, tileCopy);
#else
				CAL_CntrlLaunchCKernel(cntrl_GPU, UpdateEdgeD, thUpdateEdgesUD, stream_D,tileCopy, tileHaloD, tileMat);
				CAL_CntrlLaunchCKernel(cntrl_GPU, CopyEdgeD, thCopyEdgesUD, stream_D, tileHaloD, tileCopy);				
#endif				
				//Borde L
#ifdef SYN
				CAL_CntrlLaunch(cntrl_GPU, UpdateEdgeL, thUpdateEdgesLR, tileCopy, tileHaloL, tileMat);
				CAL_CntrlLaunch(cntrl_GPU, CopyEdgeL, thCopyEdgesLR, tileHaloL, tileCopy);
#else
				CAL_CntrlLaunchCKernel(cntrl_GPU, UpdateEdgeL, thUpdateEdgesLR, stream_L, tileCopy, tileHaloL, tileMat);
				CAL_CntrlLaunchCKernel(cntrl_GPU, CopyEdgeL, thCopyEdgesLR, stream_L, tileHaloL, tileCopy);				
#endif				
				//Borde R
#ifdef SYN
				CAL_CntrlLaunch(cntrl_GPU, UpdateEdgeR, thUpdateEdgesLR, tileCopy, tileHaloR, tileMat);
				CAL_CntrlLaunch(cntrl_GPU, CopyEdgeR, thCopyEdgesLR, tileHaloR, tileCopy);
#else
				CAL_CntrlLaunchCKernel(cntrl_GPU, UpdateEdgeR, thUpdateEdgesLR, stream_R, tileCopy, tileHaloR, tileMat);
				CAL_CntrlLaunchCKernel(cntrl_GPU, CopyEdgeR, thCopyEdgesLR, stream_R, tileHaloR, tileCopy);				
#endif
				//Computación gruesa
#ifdef SYN
				CAL_CntrlLaunch(cntrl_GPU, UpdateThick, thUpdateThick, tileCopy, tileMat);
				CAL_CntrlLaunch(cntrl_GPU, CopyThick, thCopyThick, tileMat, tileCopy);
				cudaDeviceSynchronize();
#else
				CAL_CntrlWaitStream(&cntrl_GPU, stream_U); CAL_CntrlWaitStream(&cntrl_GPU, stream_D);
				CAL_CntrlWaitStream(&cntrl_GPU, stream_L); CAL_CntrlWaitStream(&cntrl_GPU, stream_R);
				CAL_CntrlLaunchCKernel(cntrl_GPU, UpdateThick, thUpdateThick, stream_Mat, tileCopy, tileMat);
				CAL_CntrlLaunchCKernel(cntrl_GPU, CopyThick, thCopyThick, stream_Mat, tileMat, tileCopy);
#endif					

				/*Loop de computación*/
				for (int loopIndex = 0; loopIndex < numIter-1; loopIndex++) {				//******************************* LOOP
				
					/*Traerme todos los bordes al Host */
#ifdef SYN
					CAL_CntrlMoveFrom(&cntrl_GPU, (HitTile*)&tileHaloU); CAL_CntrlMoveFrom(&cntrl_GPU, (HitTile*)&tileHaloD);
					CAL_CntrlMoveFrom(&cntrl_GPU, (HitTile*)&tileHaloL); CAL_CntrlMoveFrom(&cntrl_GPU, (HitTile*)&tileHaloR);
#else
					CAL_CntrlMoveFromAsyn(&cntrl_GPU, (HitTile*)&tileHaloU, stream_U); CAL_CntrlMoveFromAsyn(&cntrl_GPU, (HitTile*)&tileHaloD, stream_D);
					CAL_CntrlMoveFromAsyn(&cntrl_GPU, (HitTile*)&tileHaloL, stream_L); CAL_CntrlMoveFromAsyn(&cntrl_GPU, (HitTile*)&tileHaloR, stream_R);
#endif					
			
					CAL_CntrlDestroy(&cntrl_GPU);							//*****************
TimerStop(tParcial);//ppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp

				
					hit_tileUpdateToAncestor( &tileHaloU ); hit_tileUpdateToAncestor( &tileHaloD ); 
					hit_tileUpdateToAncestor( &tileHaloL ); hit_tileUpdateToAncestor( &tileHaloR );

					/*Patrón de comunicación*/
					hit_patternDo( pat );
				
					/*Movimiento de Halos*/
					hit_tileUpdateFromAncestor( &tileHaloU ); hit_tileUpdateFromAncestor( &tileHaloD );
					hit_tileUpdateFromAncestor( &tileHaloL ); hit_tileUpdateFromAncestor( &tileHaloR );
TimerContinue(tParcial);//ppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp				

					CAL_CntrlCreate(&cntrl_GPU, CAL_CNTRL_GPU, GPU);		//*****************
#ifdef SYN
					CAL_CntrlMoveTo(&cntrl_GPU,(HitTile*)&tileHaloU); CAL_CntrlMoveTo(&cntrl_GPU,(HitTile*)&tileHaloD);
					CAL_CntrlMoveTo(&cntrl_GPU,(HitTile*)&tileHaloL); CAL_CntrlMoveTo(&cntrl_GPU,(HitTile*)&tileHaloR);					
					cudaDeviceSynchronize();
#else
					CAL_CntrlMoveToAsyn(&cntrl_GPU,(HitTile*)&tileHaloU, stream_U); CAL_CntrlMoveToAsyn(&cntrl_GPU,(HitTile*)&tileHaloD, stream_D);
					CAL_CntrlMoveToAsyn(&cntrl_GPU,(HitTile*)&tileHaloL, stream_L); CAL_CntrlMoveToAsyn(&cntrl_GPU,(HitTile*)&tileHaloR, stream_R);					
					CAL_CntrlWaitStream(&cntrl_GPU, stream_Mat);
#endif										
					/*Cómputo de los bordes del grueso*/
						//esperamos a que termine el grueso de la iteración anterior
					//Borde U
#ifdef SYN
					CAL_CntrlLaunch(cntrl_GPU, UpdateEdgeU, thUpdateEdgesUD, tileCopy, tileHaloU, tileMat);
					CAL_CntrlLaunch(cntrl_GPU, CopyEdgeU, thCopyEdgesUD, tileHaloU, tileCopy);
#else										
					CAL_CntrlLaunchCKernel(cntrl_GPU, UpdateEdgeU, thUpdateEdgesUD, stream_U, tileCopy, tileHaloU, tileMat);					
					CAL_CntrlLaunchCKernel(cntrl_GPU, CopyEdgeU, thCopyEdgesUD, stream_U, tileHaloU, tileCopy);					
#endif					
					//Borde D
#ifdef SYN
					CAL_CntrlLaunch(cntrl_GPU, UpdateEdgeD, thUpdateEdgesUD, tileCopy, tileHaloD, tileMat);
					CAL_CntrlLaunch(cntrl_GPU, CopyEdgeD, thCopyEdgesUD, tileHaloD, tileCopy);
#else
					CAL_CntrlLaunchCKernel(cntrl_GPU, UpdateEdgeD, thUpdateEdgesUD, stream_D, tileCopy, tileHaloD, tileMat);
					CAL_CntrlLaunchCKernel(cntrl_GPU, CopyEdgeD, thCopyEdgesUD, stream_D, tileHaloD, tileCopy);					
#endif					
					//Borde L
#ifdef SYN
					CAL_CntrlLaunch(cntrl_GPU, UpdateEdgeL, thUpdateEdgesLR, tileCopy, tileHaloL, tileMat);
					CAL_CntrlLaunch(cntrl_GPU, CopyEdgeL, thCopyEdgesLR, tileHaloL, tileCopy);
#else
					CAL_CntrlLaunchCKernel(cntrl_GPU, UpdateEdgeL, thUpdateEdgesLR, stream_L, tileCopy, tileHaloL, tileMat);
					CAL_CntrlLaunchCKernel(cntrl_GPU, CopyEdgeL, thCopyEdgesLR, stream_L, tileHaloL, tileCopy);					
#endif
					//Borde R
#ifdef SYN
					CAL_CntrlLaunch(cntrl_GPU, UpdateEdgeR, thUpdateEdgesLR, tileCopy, tileHaloR, tileMat);
					CAL_CntrlLaunch(cntrl_GPU, CopyEdgeR, thCopyEdgesLR, tileHaloR, tileCopy);
#else
					CAL_CntrlLaunchCKernel(cntrl_GPU, UpdateEdgeR, thUpdateEdgesLR, stream_R, tileCopy, tileHaloR, tileMat);
					CAL_CntrlLaunchCKernel(cntrl_GPU, CopyEdgeR, thCopyEdgesLR, stream_R, tileHaloR, tileCopy);					
#endif					
					//Computación gruesa
#ifdef SYN					
					CAL_CntrlLaunch(cntrl_GPU, UpdateThick, thUpdateThick, tileCopy, tileMat);
					CAL_CntrlLaunch(cntrl_GPU, CopyThick, thCopyThick, tileCopy, tileMat);
					cudaDeviceSynchronize();
#else
					CAL_CntrlWaitStream(&cntrl_GPU, stream_U); CAL_CntrlWaitStream(&cntrl_GPU, stream_D);
					CAL_CntrlWaitStream(&cntrl_GPU, stream_L); CAL_CntrlWaitStream(&cntrl_GPU, stream_R);
					CAL_CntrlLaunchCKernel(cntrl_GPU, UpdateThick, thUpdateThick, stream_Mat, tileCopy, tileMat);
					CAL_CntrlLaunchCKernel(cntrl_GPU, CopyThick, thCopyThick, stream_Mat, tileCopy, tileMat);					

#endif

				}//loop													//************************		END LOOP 

#ifdef SYN			
				/*Esperamos al grueso*/
				cudaDeviceSynchronize();
#else				
				CAL_CntrlWaitStream(&cntrl_GPU, stream_Mat);
#endif

				/*Obtenemos los resultados desde copia*/ 
				CAL_CntrlDetach(&cntrl_GPU, (HitTile*)&tileMat);				
				CAL_CntrlDetach(&cntrl_GPU, (HitTile*)&tileCopy);				
				CAL_CntrlDestroy(&cntrl_GPU);
TimerStop(tParcial);//ppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp
TimerStop(tTotal);	//ttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
				/*Finalización de relojes*/
				printf("----> Fils, Cols,iter: %d,%d,%d\n", rows, columns, numIter );
				printf("----> Total Clock %.8lf\n", TimerGetTime(tTotal));
				printf("----> Parcial Clock %.8lf\n\n", TimerGetTime(tParcial));
				TimerDestroy(tTotal);

					

#ifdef DEBUG
for (int i =0;i < rows; i++)
				{for (int j =0;j < columns; j++){
						printf(" %f ", (float) hit_tileElemAt2(tileMat,i,j));
						}printf("\n");}	
printf("\n");						
for (int i =0;i < rows; i++)
				{for (int j =0;j < columns; j++){
						printf("%f ", (float) hit_tileElemAt2(tileCopy,i,j));
						}printf("\n");}											
#endif
				
				/*Liberar recursos/estructuras*/
				hit_tileFree(tileMat); hit_tileFree(tileCopy);
				hit_tileFree(tileHaloU); hit_tileFree(tileHaloD);
				hit_tileFree(tileHaloL); hit_tileFree(tileHaloR);
				hit_patternFree( &pat );
				cudaStreamDestroy ( stream_U );		cudaStreamDestroy ( stream_D );
				cudaStreamDestroy ( stream_L );		cudaStreamDestroy ( stream_R );
				cudaStreamDestroy ( stream_Mat );	cudaStreamDestroy ( stream_Co );
			}//if ( hit_layImActive( matLayout ) )


	
		/*Liberar recursos/estructuras*/
		hit_tileFree(matrix);
		hit_topFree( topo );	//Topología	
		hit_layFree( matLayout );
		
		/*Finalizacion*/
        CAL_CntrlFinish();      //Controlador
        //hit_comFinalize();	//comunicadores (como es posible que sin comentar de error???)
        
    /*Retorno*/    
	return 0;
}
