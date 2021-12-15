#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "CAL.h"

#define SEED 6834723

	/*Tipo de datos*/
	hit_tileNewType( float );
	hit_ktileNewType( float );

	/* A. GPU Kernel charazterizations */
	CAL_KERNEL_GPU_CHAR_STATIC(Copy, 2, full, low, low);
	CAL_KERNEL_GPU_CHAR_STATIC(CopyEdgeU, 2, full, low, low);
	CAL_KERNEL_GPU_CHAR_STATIC(Update, 2, medium, medium, medium);
	CAL_KERNEL_GPU_CHAR_STATIC(UpdateEdgeU, 2, medium, medium, medium);

	/* B.1 GPU Kernel implementation */
	CAL_KERNEL(Copy, dGPU, KHitTile_float dst, KHitTile_float src){
		int row = threadId.y;
		int col = threadId.x;
		hit_tileElemAtNoStride(dst, 2, row, col) = hit_tileElemAtNoStride(src, 2, row, col);
	}//Copy

	CAL_KERNEL(CopyEdgeU, dGPU, KHitTile_float edgeU, KHitTile_float src){
		int row = threadId.y + 1;
		int col = threadId.x;
		hit_tileElemAtNoStride(edgeU, 2, 0, col) = hit_tileElemAtNoStride(src, 2, row, col);

	}//CopyEdgeU
		

	/* B.2 GPU Kernel implementation */
	CAL_KERNEL(Update, dGPU, KHitTile_float dst, KHitTile_float src){
		int row = threadId.y + 1;
		int col = threadId.x + 1;
		hit_tileElemAtNoStride(dst, 2, row, col) = ( 
				hit_tileElemAtNoStride(src, 2, row - 1, col    ) +
				hit_tileElemAtNoStride(src, 2, row + 1, col    ) +
				hit_tileElemAtNoStride(src, 2, row    , col - 1) +
				hit_tileElemAtNoStride(src, 2, row    , col + 1) ) / 4;
	}//Update

	CAL_KERNEL(UpdateEdgeU, dGPU, KHitTile_float dst,  KHitTile_float src){
		int row = threadId.y + 1;
		int col = threadId.x + 1;
	
			hit_tileElemAtNoStride(dst, 2, row, col) = 66.34;
						
	}//UpdateEdgeU

	/* C. Defining kernel prototypes */
	CAL_KERNEL_PROTO( Copy,                                               	// Name
					1, dGPU,                                   				// Implementations
					2, OUT, HitTile_float, dst, IN, HitTile_float, src
					);
	CAL_KERNEL_PROTO( CopyEdgeU,                                               	// Name
                  1, dGPU,                                   					// Implementations
                  2, OUT, HitTile_float, edgeU, IN, HitTile_float, src
					);
	CAL_KERNEL_PROTO( Update,												// Name
					1, dGPU,												// Implementations
					2, OUT, HitTile_float, dst, IN, HitTile_float, src
					);
	CAL_KERNEL_PROTO( UpdateEdgeU,                                               // Name
					1, dGPU,        						                         // Implementations
					2, OUT, HitTile_float, dst, IN, HitTile_float, src 
					);


	/*Inicialización de matrices*/
	void initMatrix( HitTile_float tileMat ) {																//**********************

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

	/*hit_shapeExpand. Debería de no tener que estar aquí*/
	HitShape hit_shapeExpand(HitShape shape,int dims,int offset){							//**********************

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
	HitShape hit_shapeDimExpand(HitShape shape,int dim, int position, int offset ){				//**********************

		// Only works for Signature Shape.
		if(	hit_shapeType(shape) == HIT_CSR_SHAPE ) return HIT_SHAPE_NULL;

		HitShape res = shape;

		if ( position == HIT_SHAPE_BEGIN ) hit_shapeSig(res,dim).begin += offset;
		else hit_shapeSig(res,dim).end += offset;

		return res;
	}//hit_shapeDimExpand


/* **********************************************************************
*************												************* 
****			Main program to perform cellular automata 			 ****
*************												*************
* **********************************************************************/
int main(int argc, char* argv[]) {

#ifdef DEBUG
        setbuf(stderr, NULL);
        setbuf(stdout, NULL);
#endif

		/*Comunicadores infernales*/
		hit_comInit( &argc, &argv );
		
        // 1. Init Controllers library									//**********************
        omp_set_nested(1);
        omp_set_num_threads(3);
        #pragma omp parallel 
        {
        #pragma omp single 
         {

		/*2. Taking arguments*/											//**********************
		if ( argc != 4 ) {
			fprintf(stderr, "Usage: %s <matrixSize> <numStages> <GPU>\n", argv[0] );
			exit( EXIT_FAILURE );
		}//if
		int SIZE = atoi(argv[1]);
		int STAGES = atoi(argv[2]);
		int GPU = atoi(argv[3]);

		/*Tiles y tiempos y semilla*/									//**********************
		HitTile_float mat, tileMat;
		HitTile_float copy, tileCopy;
		HitTile_float  tileHaloU, tileHaloD, tileHaloL, tileHaloR;
		Timer tTotal;
		Timer tKernel;
		TimerCreate(tTotal);
		TimerCreate(tKernel);
		srand( SEED );
		
		/*Creamos topología*/
		HitTopology topo = hit_topology( plug_topArray2DComplete );
		
        /*3. Declare and initialize full matrices and domains*/			//**********************
		hit_tileDomain ( &mat, float, 2, SIZE, SIZE );
		hit_tileDomain ( &copy, float, 2, SIZE, SIZE );
		hit_tileAlloc( &mat );
		//hit_tileAlloc( &copy );

		
		/*Creamos el shape y el layout*/
		HitShape parallelShape = hit_tileShape( mat );
		parallelShape = hit_shapeExpand( parallelShape, 2, -1 );
		HitLayout matLayout = hit_layout( plug_layBlocks, topo, parallelShape );
		
		/*Si soy proceso activo...*/
		if ( hit_layImActive( matLayout ) ) {							//**********************

			/*Inicialización de Tiles*/									//**********************
			for ( int i=0; i<SIZE; i++ )
				for ( int j=0; j<SIZE; j++ )
					hit_tileElemAt( mat, 2, i, j ) = 0.0;
			for ( int j=0; j<SIZE; j++ ) {
				hit_tileElemAt( mat, 2, 0, j ) = 1;
				hit_tileElemAt( mat, 2, SIZE-1, j ) = 2;}		
			for ( int i=0; i<SIZE; i++ ) {
				hit_tileElemAt( mat, 2, i, 0 ) = 3;
				hit_tileElemAt( mat, 2, i, SIZE-1 ) = 4;}
			

							//HitTile_float tileMat, tileCopy;
							HitShape expandedShape = hit_shapeDimExpand( hit_layShape(matLayout), 0, HIT_SHAPE_BEGIN, -1 );
							expandedShape = hit_shapeDimExpand( expandedShape, 0, HIT_SHAPE_END, 1 );
							expandedShape = hit_shapeDimExpand( expandedShape, 1, HIT_SHAPE_BEGIN, -1 );
							expandedShape = hit_shapeDimExpand( expandedShape, 1, HIT_SHAPE_END, 1 );
							hit_tileSelect( &tileMat, &mat, expandedShape );
							hit_tileAlloc( &tileMat );
							//
							hit_tileSelect( &tileCopy, &tileMat, HIT_SHAPE_WHOLE );
							hit_tileAlloc( &tileCopy );

							for ( int i=0; i<SIZE; i++ )
								for ( int j=0; j<SIZE; j++ )
									hit_tileElemAt( tileMat, 2, i, j ) = 0.0;
							for ( int j=0; j<SIZE; j++ ) {
								hit_tileElemAt( tileMat, 2, 0, j ) = 1;
								hit_tileElemAt( tileMat, 2, SIZE-1, j ) = 2;}		
							for ( int i=0; i<SIZE; i++ ) {
								hit_tileElemAt( tileMat, 2, i, 0 ) = 3;
								hit_tileElemAt( tileMat, 2, i, SIZE-1 ) = 4;}
						
							/*Patrón de comunicación y primera comunicación*/
							HitPattern pat = hit_patternLayRedistribute(matLayout, matLayout, &tileCopy, &tileMat, HIT_FLOAT);
							hit_patternDo( pat );

							/*Declaración y Selección de halos (hacemos duplicado de halos)*/
							HitShape uShape; 
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

	
			cudaDeviceSynchronize();
			TimerStart(tTotal);

			// 4. Init computation threads								//**********************
			CALThread threads, update;
			CALThreadInit(threads, 2, SIZE, SIZE);
			CALThreadInit(update, 2, SIZE-2, SIZE-2);
			CALThread thUpdateEdgesUD, thCopyEdgesUD; 
			CALThread thUpdateEdgesLR, thCopyEdgesLR;
			CALThread thUpdateThick, thCopyThick;
			CALThreadInit(thUpdateEdgesUD, 2, SIZE - 2, SIZE - 2); CALThreadInit(thCopyEdgesUD, 2, SIZE - 2, SIZE - 2); 
			CALThreadInit(thUpdateEdgesLR, 2, SIZE - 2, 1); CALThreadInit(thCopyEdgesLR, 2, SIZE, 1);							
			CALThreadInit(thUpdateThick, 2, SIZE - 2, SIZE - 2); CALThreadInit(thCopyThick, 2, SIZE - 2, SIZE - 2);
		

			// 5. Create controller object
			CALCntrl comm;
			CAL_CntrlCreate(&comm, CAL_CNTRL_GPU, GPU);

			// 6. Attach data structures to the controllers
			cudaStream_t stream_U, stream_D, stream_L, stream_R, stream_Mat, stream_Co;	
			cudaStreamCreate ( &stream_U );		cudaStreamCreate ( &stream_D );
			cudaStreamCreate ( &stream_L );		cudaStreamCreate ( &stream_R );
			cudaStreamCreate ( &stream_Mat );	cudaStreamCreate ( &stream_Co );		        
			CAL_CntrlAttachAsyn(&comm, (HitTile*)&mat, stream_Mat);
			CAL_CntrlAttachAsyn(&comm, (HitTile*)&copy, stream_Co);
			
									CAL_CntrlAttachAsyn(&comm, (HitTile*)&tileMat, stream_Mat);
									CAL_CntrlAttachAsyn(&comm, (HitTile*)&tileCopy, stream_Co);									
									CAL_CntrlAttachAsyn(&comm,(HitTile*)&tileHaloU, stream_U); CAL_CntrlAttachAsyn(&comm,(HitTile*)&tileHaloD, stream_D);
									CAL_CntrlAttachAsyn(&comm,(HitTile*)&tileHaloL, stream_L); CAL_CntrlAttachAsyn(&comm,(HitTile*)&tileHaloR, stream_R);
									//Borde U
									CAL_CntrlLaunchCKernel(comm, UpdateEdgeU, thUpdateEdgesUD, stream_U, tileCopy, tileMat); 
									CAL_CntrlLaunchCKernel(comm, CopyEdgeU, thCopyEdgesUD, stream_U, tileHaloU, tileCopy);
			
				


			cudaDeviceSynchronize();

			// 7. Launch the task: invoking the kerneli
			for (int i = 0; i < STAGES; i++){
				// Update copy
				CAL_CntrlLaunchCKernel(comm, Copy, threads, stream_Mat, copy, mat);
				//CAL_CntrlLaunchCKernel(comm, Copy, threads, stream_Mat, tileCopy, tileMat);
		
				// Compute iteration
				CAL_CntrlLaunchCKernel(comm, Update, update, stream_Mat,mat, copy);
				//CAL_CntrlLaunchCKernel(comm, Update, update, stream_Mat,tileMat, tileCopy);
			}

			// 8. Copy result from device memory to host memory
			CAL_CntrlDetach(&comm, (HitTile*)&mat);
			CAL_CntrlDetach(&comm, (HitTile*)&tileMat);
			CAL_CntrlDetach(&comm, (HitTile*)&tileHaloU);
						
			// 9. Destroy the controller  y otras cosas
			CAL_CntrlDestroy(&comm);
			hit_patternFree( &pat );
			printf("HOLA caracola: %lf %lf \n ", hit_tileElemAt( tileHaloU, 2, 0,1), hit_tileElemAt( tileHaloU, 2, 0,2) );

		}//if															//********************** if

										


		// 10. Times
		cudaDeviceSynchronize();
		TimerStop(tTotal);
		printf("SIZE %d %d\n", SIZE, STAGES );
        printf("Clock Main %.8lf\n", TimerGetTime(tTotal));

        // 11. Calculate NORM
        double resultado=0,suma=0;
        for (int i=0; i<SIZE; i++){
			for (int j=0; j<SIZE; j++) {
				// #ifdef DEBUG
                printf("%lf ", hit_tileElemAt( mat, 2, i, j ));
				//#endif
				suma += pow(  hit_tileElemAt( mat, 2, i, j ) ,2);
		}printf("\n");}
        printf("\n ----------------------- NORM ----------------------- \n");
        printf("\n Acumulated sum: %lf",suma);

        resultado=sqrt( suma );
        printf("\n Result: %lf \n",resultado);

        printf("\n ---------------------------------------------------- \n");


        for (int i=0; i<SIZE; i++){
			for (int j=0; j<SIZE; j++) {
				// #ifdef DEBUG
                printf("%lf ", hit_tileElemAt( tileMat, 2, i, j ));
				//#endif
				suma += pow(  hit_tileElemAt( tileMat, 2, i, j ) ,2);
		}printf("\n");}

		//printf("Tiempo kernels: %.8lf\n",TimerGetTime(tKernel));
	
		// 12. Free data structures
		hit_tileFree(mat);
		TimerDestroy(tTotal);
		TimerDestroy(tKernel);
 
		// 13. Finish the controller library
		CAL_CntrlFinish();

	return 0;
}
