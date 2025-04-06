
/**
 * @file Ctrl_ALB.h
 * @author Trasgo Group
 * @brief Prototypes for ALB functions and structs for ALB data types.
 * @version 1.0
 * @date 2024-05-03
 *
 * @copyright This software is provided to enhance knowledge and encourage progress in the scientific
 * community. It should be used only for research and educational purposes. Any reproduction
 * or use for commercial purpose, public redistribution, in source or binary forms, with or
 * without modifications, is NOT ALLOWED without the previous authorization of the copyright
 * holder. The origin of this software must not be misrepresented; you must not claim that you
 * wrote the original software. If you use this software for any purpose (e.g. publication),
 * a reference to the software package and the authors must be included.
 *
 * @copyright THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @copyright Copyright (c) 2007-2020, Trasgo Group, Universidad de Valladolid.
 * All rights reserved.
 *
 * @copyright More information on http://trasgo.infor.uva.es/
 */


#include "Core/Ctrl_ALB.h"


// FUNCTIONS AND STRUCTS RELATED TO ALB
static inline void printHitShape2(HitShape sh){
	HitSig sig;

	printf("HitShape(dims=%d)\n",hit_sshapeDims(sh));
	for(int dim=0; dim<hit_sshapeDims(sh); dim++){
		sig = hit_shapeSig(sh, dim);
		printf("[%d] Dim %d\n", hit_Rank, dim);
		printf("\t[%d] HitSig(begin=%d, end=%d, stride=%d, card=%d)\n", hit_Rank, sig.begin, sig.end, sig.stride, hit_sigCard(sig));
		printf("\t[%d] ", hit_Rank);
		for(int i=sig.begin; i<=sig.end; i+=sig.stride) printf("%d ", i);

		printf("\n");
	}
}

/*
* @tomascalest May 2024
* 	allocateTileData: Allocates space for tile data
*/
void allocateTileData(TileData *data, TileCommon dataCommon, int dims){
	(*data).tileBorderOutDev = malloc(sizeof(HitTile_float) * dims * 2);
	(*data).tileBorderIn = malloc(sizeof(HitTile_float) * dataCommon.numBorders);
}

/*
* @tomascalest May 2024
* 	freeTileData: Frees the space used by tile data
*/
void freeTileData(TileData *data){
	free((*data).tileBorderOutDev);
	free((*data).tileBorderIn);
}

/*
* @tomascalest May 2024
* 	selectIOTiles: Computes the io_tiles and save them inside a TileCommon object
*	This function is extracted from stencilComputation.
*/
void selectIOTiles(TileData *tileData, TileData *tileDataCopy, TileCommon *dataCommon, int* borderLow, int* borderHigh){
	// We get the number of dimensions from shape and 
	// the root tile from tileMat (should be the same for tileCopy)
	int dims = hit_shapeDims(hit_tileShape(tileData->tileMat));
	HitTile *globalMat = hit_tileRoot(&(tileData->tileMat));

	// We get the io_tiles by first getting a shape that has all borders and halos
	// and then removing the halos from it
	/* ELIMINATE BORDERS EXCEPT IF THEY ARE GLOBAL FOR IO SELECTION */
	HitShape io_shape = hit_tileShape(tileData->tileMat);
	for (int i = 0; i < dims; i++) {
		/* DIM i FIRST BORDER IS NOT MINE */
		if (!hit_sigIn(hit_shapeSig(io_shape, i), hit_tileDimBegin(*globalMat, i))) {
			io_shape = hit_shapeTransform(io_shape, i, HIT_SHAPE_BEGIN, borderLow[i]);
		}
		/* DIM i LAST BORDER IS NOT MINE */
		if (!hit_sigIn(hit_shapeSig(io_shape, i), hit_tileDimEnd(*globalMat, i))) {
			io_shape = hit_shapeTransform(io_shape, i, HIT_SHAPE_END, -borderHigh[i]);
		}
	}

	// We select the coords with the previously computed shape
	hit_tileSelectArrayCoords(&(tileData->io_tile), &(tileData->tileMat), io_shape);
	hit_tileSelectArrayCoords(&(tileDataCopy->io_tile), &(tileDataCopy->tileMat), io_shape);
}

/*
* @tomascalest May 2024
* 	computeCommonData: Computes data that is common to both tiles
*	This function is extracted and adapted from stencilComputation.
*/
void computeCommonData(TileCommon *dataCommon, HitShape shp, HitShape shpLayout, HitLayout lay, HitTile_float stencil, int dims, float stencilData[], int *borderLow, int *borderHigh){
	// 4.1. STOP IF THERE IS NOT ENOUGH INNER DATA FOR A BORDER
	for (int i = 0; i < dims; i++) {
		if (hit_shapeSigCard(shpLayout, i) < borderLow[i] ||
			hit_shapeSigCard(shpLayout, i) < borderHigh[i]) {
			if (hit_Rank == 0) {
				fprintf(stderr, "\nError: Not enough data after partition, too many processes in a topology axis\n\n");
				MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
				exit(EXIT_FAILURE);
			}
		}
	}

	/* 4.3. BORDER SHAPES: IN/OUT */
	dataCommon->numBorders = (int)pow(3, dims);
	
	for (int i = 0; i < dataCommon->numBorders; i++)
		*(dataCommon->borderInActive+i) = 0;

	// TRAVERSE THE STENCIL TO DETECT ACTIVE AND INACTIVE BORDERS DUE TO WEIGHTS
	int indeces[dims];
	int displacement = 0;
	for (int j = 0; j < dims; j++)
		indeces[j] = 0;
	int endAnalysis = 0;
	while (!endAnalysis) {
		// CHECK IF THERE IS A WEIGHT IN THE STENCIL POSITION
		if (stencilData[displacement] != 0) {
			// ACTIVE BORDER, COMPUTE ITS NUMBER TO RAISE THE FLAG
			int acum   = 1;
			int border = 0;
			for (int j = dims - 1; j >= 0; j--) {
				if (indeces[j] > borderLow[j])
					border += 2 * acum;
				else if (indeces[j] == borderLow[j])
					border += acum;
				acum *= 3;
			}
			*(dataCommon->borderInActive+border) = 1;
		}
		// ADVANCE TO THE NEXT STENCIL POSITION
		displacement++;
		for (int k = dims - 1; k >= 0; k--) {
			indeces[k]++;
			if (k == 0 && indeces[0] == hit_tileDimCard(stencil, 0)) endAnalysis = 1;
			if (indeces[k] >= hit_tileDimCard(stencil, k))
				indeces[k] = 0;
			else
				break;
		}
	}
	// ALWAYS SKIP FALSE BORDER: TILE INNER
	*(dataCommon->borderInActive + (dataCommon->numBorders / 2)) = 0;

	HitShape shpBorderIn[dataCommon->numBorders];
	HitShape shpBorderOut[dataCommon->numBorders];
	HitRanks shiftsIn[dataCommon->numBorders];
	HitRanks shiftsOut[dataCommon->numBorders];
	// BUILD BORDER SHAPES AND THE SHORTCUTS OF THE NEIGHBOR SHIFTS
	for (int i = 0; i < dataCommon->numBorders; i++) {
		shiftsIn[i]  = HIT_RANKS_NULL;
		shiftsOut[i] = HIT_RANKS_NULL;

		// NON-ACTIVE BORDERS, NULL SHAPES, NULL RANKS
		if (!(*(dataCommon->borderInActive + i))) {
			shpBorderIn[i]  = HIT_SHAPE_NULL;
			shpBorderOut[i] = HIT_SHAPE_NULL;
			continue;
		}
		shpBorderIn[i]  = shpLayout;
		shpBorderOut[i] = shpLayout;

		// EXTRACT RANKS FOR THIS BORDER
		int digits = i;
		for (int j = 0; j < dims; j++) {
			shiftsIn[i].rank[j]  = digits % 3 - 1;
			shiftsOut[i].rank[j] = -shiftsIn[i].rank[j];
			digits /= 3;

			// SHAPE IN
			if (shiftsIn[i].rank[j] == -1) {
				shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_FIRST, borderLow[j]);
				shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_MOVE, -borderLow[j]);
			} else if (shiftsIn[i].rank[j] == 1) {
				shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_LAST, borderHigh[j]);
				shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_MOVE, borderHigh[j]);
			}

			// SHAPE OUT (REVERSERD TARGET)
			if (shiftsIn[i].rank[j] == -1) {
				shpBorderOut[i] = hit_shapeTransform(shpBorderOut[i], j, HIT_SHAPE_LAST, borderLow[j]);
			} else if (shiftsIn[i].rank[j] == 1) {
				shpBorderOut[i] = hit_shapeTransform(shpBorderOut[i], j, HIT_SHAPE_FIRST, borderHigh[j]);
			}
		}
	}
		

	// DEACTIVATE BORDER COMMs DUE TO LACK OF NEIGHBOR
	for (int i = 0; i < dataCommon->numBorders; i++) {
		// SKIP EMPTY BORDERS
		if (!(*(dataCommon->borderInActive + i))) continue;
		// LOCATE NEIGHBOR IN THE LAYOUT GRID
		HitRanks neighIn  = hit_layNeighborN(lay, shiftsIn[i]);
		HitRanks neighOut = hit_layNeighborN(lay, shiftsOut[i]);

		// IF NEIGHBORS DOES NOT EXIST, DEACTIVATE BORDER COMMs.
		if (neighIn.rank[0] == -1 && neighOut.rank[0] == -1) {
			*(dataCommon->borderInActive + i) = 0;
		}
	}
	dataCommon->num_tiles_total = 1 + 1 + 2 * dataCommon->numBorders + dims * 2;
	dataCommon->dims = dims;
}

/*
* @tomascalest May 2024
* 	computeSingleTileData: Computes a tile
*	This function is extracted and adapted from stencilComputation.
*/
void computeSingleTileData(TileData *tileData, TileCommon *dataCommon, PCtrl comm, HitShape shp, HitShape shpLayout, HitLayout lay, HitTile_float stencil, int dims, float stencilData[], int *borderLow, int *borderHigh, HitTile_float *globalMat){
	// 4.1. STOP IF THERE IS NOT ENOUGH INNER DATA FOR A BORDER
	for (int i = 0; i < dims; i++) {
		if (hit_shapeSigCard(shpLayout, i) < borderLow[i] ||
			hit_shapeSigCard(shpLayout, i) < borderHigh[i]) {
			if (hit_Rank == 0) {
				fprintf(stderr, "\nError: Not enough data after partition, too many processes in a topology axis\n\n");
				MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
				exit(EXIT_FAILURE);
			}
		}
	}

	/* 4.3. BORDER SHAPES: IN/OUT */

	// TRAVERSE THE STENCIL TO DETECT ACTIVE AND INACTIVE BORDERS DUE TO WEIGHTS
	int indeces[dims];
	int displacement = 0;
	for (int j = 0; j < dims; j++)
		indeces[j] = 0;
	int endAnalysis = 0;
	while (!endAnalysis) {
		// CHECK IF THERE IS A WEIGHT IN THE STENCIL POSITION
		if (stencilData[displacement] != 0) {
			// ACTIVE BORDER, COMPUTE ITS NUMBER TO RAISE THE FLAG
			int acum   = 1;
			int border = 0;
			for (int j = dims - 1; j >= 0; j--) {
				if (indeces[j] > borderLow[j])
					border += 2 * acum;
				else if (indeces[j] == borderLow[j])
					border += acum;
				acum *= 3;
			}
		}
		// ADVANCE TO THE NEXT STENCIL POSITION
		displacement++;
		for (int k = dims - 1; k >= 0; k--) {
			indeces[k]++;
			if (k == 0 && indeces[0] == hit_tileDimCard(stencil, 0)) endAnalysis = 1;
			if (indeces[k] >= hit_tileDimCard(stencil, k))
				indeces[k] = 0;
			else
				break;
		}
	}
	
	
	
	HitShape shpBorderIn[dataCommon->numBorders];
	HitShape shpBorderOut[dataCommon->numBorders];
	HitRanks shiftsIn[dataCommon->numBorders];
	HitRanks shiftsOut[dataCommon->numBorders];
	// BUILD BORDER SHAPES AND THE SHORTCUTS OF THE NEIGHBOR SHIFTS
	for (int i = 0; i < dataCommon->numBorders; i++) {
		shiftsIn[i]  = HIT_RANKS_NULL;
		shiftsOut[i] = HIT_RANKS_NULL;

		// NON-ACTIVE BORDERS, NULL SHAPES, NULL RANKS
		if (!(*(dataCommon->borderInActive + i))) {
			shpBorderIn[i]  = HIT_SHAPE_NULL;
			shpBorderOut[i] = HIT_SHAPE_NULL;
			continue;
		}
		shpBorderIn[i]  = shpLayout;
		shpBorderOut[i] = shpLayout;

		// EXTRACT RANKS FOR THIS BORDER
		int digits = i;
		for (int j = 0; j < dims; j++) {
			shiftsIn[i].rank[j]  = digits % 3 - 1;
			shiftsOut[i].rank[j] = -shiftsIn[i].rank[j];
			digits /= 3;

			// SHAPE IN
			if (shiftsIn[i].rank[j] == -1) {
				shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_FIRST, borderLow[j]);
				shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_MOVE, -borderLow[j]);
			} else if (shiftsIn[i].rank[j] == 1) {
				shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_LAST, borderHigh[j]);
				shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_MOVE, borderHigh[j]);
			}

			// SHAPE OUT (REVERSERD TARGET)
			if (shiftsIn[i].rank[j] == -1) {
				shpBorderOut[i] = hit_shapeTransform(shpBorderOut[i], j, HIT_SHAPE_LAST, borderLow[j]);
			} else if (shiftsIn[i].rank[j] == 1) {
				shpBorderOut[i] = hit_shapeTransform(shpBorderOut[i], j, HIT_SHAPE_FIRST, borderHigh[j]);
			}
		}
	}
	/* 4.4. NON-OVERLAPED BORDERS IN THE INNER PART
		a) TO EXTRACT DATA FROM DEVICE WITHOUT REPLICATION
		b) TO DETERMINE THE THREADS-GRID CARDINALITIES FOR THE COMPUTING KERNELS
	*/
	HitShape shpInnerLocal = shpLayout;
	HitShape shpBorderOutDev[dims][2];
	for (int j = 0; j < dims; j++) {
		shpInnerLocal = hit_shapeTransform(shpInnerLocal, j, HIT_SHAPE_BEGIN, borderHigh[j]);
		shpInnerLocal = hit_shapeTransform(shpInnerLocal, j, HIT_SHAPE_END, -borderLow[j]);

		shpBorderOutDev[j][0] = hit_shapeTransform(shpLayout, j, HIT_SHAPE_FIRST, borderHigh[j]);
		shpBorderOutDev[j][1] = hit_shapeTransform(shpLayout, j, HIT_SHAPE_LAST, borderLow[j]);
		// TAKE OUT THE PARTS WHICH ARE OVERLAPPED WITH PREVIOUS DIMS
		for (int k = 0; k < j; k++) {
			shpBorderOutDev[j][0] = hit_shapeTransform(shpBorderOutDev[j][0], k, HIT_SHAPE_BEGIN, borderHigh[k]);
			shpBorderOutDev[j][0] = hit_shapeTransform(shpBorderOutDev[j][0], k, HIT_SHAPE_END, -borderLow[k]);
			shpBorderOutDev[j][1] = hit_shapeTransform(shpBorderOutDev[j][1], k, HIT_SHAPE_BEGIN, borderHigh[k]);
			shpBorderOutDev[j][1] = hit_shapeTransform(shpBorderOutDev[j][1], k, HIT_SHAPE_END, -borderLow[k]);
		}
		// NULL SIGNATURE
	}

	for (int j = 0; j < dims; j++) {
		for (int k = 0; k < dims; k++) {
			if (hit_shapeSig(shpBorderOutDev[j][0], k).begin > hit_shapeSig(shpBorderOutDev[j][0], k).end) {
				shpBorderOutDev[j][0] = HIT_SIG_SHAPE_NULL;
				break;
			}
		}
	}
	for (int j = 0; j < dims; j++) {
		for (int k = 0; k < dims; k++) {
			if (hit_shapeSig(shpBorderOutDev[j][1], k).begin > hit_shapeSig(shpBorderOutDev[j][1], k).end) {
				shpBorderOutDev[j][1] = HIT_SIG_SHAPE_NULL;
				break;
			}
		}
	}
	/* 4.5. BUILD TILES */


	//TO-DO SACAR DE LA FUNCION PARA LIBERAR
	//Creo que no es necesario?
	HitTile_float tileBorderOut[dataCommon->numBorders];
	HitTile_float tileCopyBorderOut[dataCommon->numBorders];


	/* 4.2. EXPANDED SHAPE */
	HitShape shpExpanded = shpLayout;
	for (int i = 0; i < dims; i++) {
		shpExpanded = hit_shapeTransform(shpExpanded, i, HIT_SHAPE_BEGIN, -borderLow[i]);
		shpExpanded = hit_shapeTransform(shpExpanded, i, HIT_SHAPE_END, borderHigh[i]);
	}

	tileData->tileMat  = Ctrl_Select(comm, float, *globalMat, shpExpanded, CTRL_SELECT_ARR_COORD); //-------------//select tileMat
	

	Ctrl_Alloc(comm, tileData->tileMat, CTRL_MEM_ALLOC_BOTH);  //---------------------//alloc tileMat
	

	tileData->tileInnerLocal    = Ctrl_Select(comm, float, tileData->tileMat, shpInnerLocal, CTRL_SELECT_ARR_COORD);

	for (int i = 0; i < dataCommon->numBorders; i++) {
		(*((tileData->tileBorderIn)+i)) = Ctrl_Select(comm, float, tileData->tileMat, shpBorderIn[i], CTRL_SELECT_ARR_COORD);   //-------------------//select
		tileBorderOut[i] = Ctrl_Select(comm, float, tileData->tileMat, shpBorderOut[i], CTRL_SELECT_ARR_COORD);  //-------------------//select
	}

	

	// clang-format off
	
	#define validShape(s) (hit_shapeDims((s)) != (-1))
	// clang-format on

	
	for (int i = 0; i < dims; i++) {
		if (validShape(shpBorderOutDev[i][0])) {
			(*((tileData->tileBorderOutDev + i * 2) + 0)) = Ctrl_Select(comm, float, tileData->tileMat, shpBorderOutDev[i][0], CTRL_SELECT_ARR_COORD);  //-------------------//select
		} else {
			(*((tileData->tileBorderOutDev + i * 2) + 0)) = *(HitTile_float *)&HIT_TILE_NULL;
		}
		if (validShape(shpBorderOutDev[i][1])) {
			(*((tileData->tileBorderOutDev + i * 2) + 1))  = Ctrl_Select(comm, float, tileData->tileMat, shpBorderOutDev[i][1], CTRL_SELECT_ARR_COORD);  //-------------------//select
		} else {
			(*((tileData->tileBorderOutDev + i * 2) + 1)) = *(HitTile_float *)&HIT_TILE_NULL;
		}


	}

	/* 4.6. BUILD DISTRIBUTED-MEMORY COMMUNICATION PATTERN */
	tileData->neighSync = hit_pattern(HIT_PAT_UNORDERED);
	for (int i = 0; i < dataCommon->numBorders; i++) {
		// SKIP EMPTY BORDERS
		if (!(*(dataCommon->borderInActive + i))) continue;

		// LOCATE NEIGHBORS IN THE LAYOUT GRID

		HitRanks neighIn  = HIT_RANKS_NULL;
		HitRanks neighOut = HIT_RANKS_NULL;

		if (*(dataCommon->borderInActive + i)) {
			neighIn  = hit_layNeighborN(lay, shiftsIn[i]);
			neighOut = hit_layNeighborN(lay, shiftsOut[i]);
		}

		hit_patternAdd(&(tileData->neighSync), hit_comSendRecv(lay,
													neighOut, &tileBorderOut[i],
													neighIn, (tileData->tileBorderIn + i),
													HIT_FLOAT));
	}			
}


/*
* @tomascalest May 2024
* 	Ctrl_ALB: This function rebalances the load of the computing nodes 
*	This function is extracted and adapted from the work of Maria Sanchez Giron in hit_alb.c.
*	When it is time to rebalance the load, we first free the tileCopy, then we create a new
*	temporary tile and redistribute the old one to it. Then we free the old tile and create a
*	new tilecopy.
*
*/
int Ctrl_ALB(TileData *tileData, TileData *tileDataCopy, TileCommon *dataCommon, HitLayout *lay, HitShape *shpLayout, HitAvg * avg, double time,
		PCtrl comm, HitShape shp, HitTile_float stencil, int dims, float stencilData[], int *borderLow, int *borderHigh,
		Ctrl_Thread *threadsInner, Ctrl_Thread *thrBorderOutDev){

	int dev = 0;
	static HitClock callClock = {HIT_CLOCK_STOPPED, -1, 0, 0, 0, 0};
	static HitClock redisClock = {HIT_CLOCK_STOPPED, -1, 0, 0, 0, 0};
	static HitLayout layoutComm;	// Communications layout. All processes active
	static HitTile_double allTimes;
	static HitTile_double avgTimes;
	static HitTile_double redisTimes;
	static MPI_Request reqAllTimes;
	static MPI_Request reqAvgTimes;
	static MPI_Request reqRedisTimes;
	static int currentALBIter;
	static int currentIter;
	static int nextALB;
	static double mediaRedisTime;
	static double lastRedisSeconds;
	HitTile_double timesAllTimes;
	HitTile_double timesAvgTimes;
	HitTile_double timesRedisTimes;
	


	if (callClock.seconds == -1){	//First call to the function, initialize layoutComm
		layoutComm = hit_layout_freeTopo( plug_layCopy, hit_topology(plug_topPlain), hit_shapeStd(1, 1) );
		currentALBIter = 0;
		currentIter = 0;
		nextALB = 0;
		mediaRedisTime = 0;
		lastRedisSeconds = 0;
	}
	else{
		#ifdef DEBUG
			hit_clockStop(callClock);
			if (hit_layImActive(*lay)) printf("Process[%d] Time between calls: %lf\n", hit_Rank, callClock.seconds);
		#endif
	}

	if (! hit_layImActive(*lay)) time = 0.0;

	#ifdef DEBUG
		printf("Process[%d] Time parameter: %lf\n", hit_Rank, time);
	#endif

	hit_avgInsertData(avg, time);
	double average = hit_avgGetAvg(*avg);

	if((average != -1) && (currentIter >= nextALB)){
		if (avg->first == 0 && avg->pointer == 0){	// first time that the data array is full
			double zero = 0;
			hit_tileDomainAlloc(&timesAllTimes, double, 1, hit_NProcs);
			hit_tileDomainAlloc(&timesAvgTimes, double, 1, hit_NProcs);
			hit_tileDomainAlloc(&timesRedisTimes, double, 1, hit_NProcs);
			hit_tileDomainAlloc(&allTimes, double, 1, hit_NProcs);
			hit_tileDomainAlloc(&avgTimes, double, 1, hit_NProcs);
			hit_tileDomainAlloc(&redisTimes, double, 1, hit_NProcs);
			hit_tileFill(&allTimes, &zero);
			hit_tileFill(&avgTimes, &zero);
			hit_tileFill(&redisTimes, &zero);
			double timePerRow;
			if (! hit_layImActive(*lay)) timePerRow = 0.0;
			else timePerRow = average / hit_tileDimCard(((tileData->tileMat)), 0);

			double redisTime = redisClock.seconds;
			hit_tileFill(&timesAllTimes, &timePerRow);

			hit_tileFill(&timesAvgTimes, &average);

			hit_tileFill(&timesRedisTimes, &redisTime);

			
			HitCom comAllTimes = hit_comAlltoall(layoutComm, &timesAllTimes, &allTimes, HIT_DOUBLE, 1);
			int okAllTimes = MPI_Ialltoall(comAllTimes.dataSend, comAllTimes.count, comAllTimes.typeSend, comAllTimes.dataRecv, comAllTimes.count, comAllTimes.typeSend, comAllTimes.comm, &reqAllTimes );
			hit_mpiTestError(okAllTimes,"Failed ialltoall send");

			HitCom comAvgTimes = hit_comAlltoall(layoutComm, &timesAvgTimes, &avgTimes, HIT_DOUBLE, 1);
			int okAvgTimes = MPI_Ialltoall(comAvgTimes.dataSend, comAvgTimes.count, comAvgTimes.typeSend, comAvgTimes.dataRecv, comAvgTimes.count, comAvgTimes.typeSend, comAvgTimes.comm, &reqAvgTimes );
			hit_mpiTestError(okAvgTimes,"Failed ialltoall send");

			HitCom comRedisTimes = hit_comAlltoall(layoutComm, &timesRedisTimes, &redisTimes, HIT_DOUBLE, 1);
			int okRedisTimes = MPI_Ialltoall(comRedisTimes.dataSend, comRedisTimes.count, comRedisTimes.typeSend, comRedisTimes.dataRecv, comRedisTimes.count, comRedisTimes.typeSend, comRedisTimes.comm, &reqRedisTimes );
			hit_mpiTestError(okRedisTimes,"Failed ialltoall send");
		} else {
			lastRedisSeconds = redisClock.seconds;
			hit_clockStart(redisClock);
			int okAllTimes = MPI_Wait( &reqAllTimes, MPI_STATUS_IGNORE );
			hit_mpiTestError(okAllTimes,"Failed ialltoall wait");
			int okAvgTimes = MPI_Wait( &reqAvgTimes, MPI_STATUS_IGNORE );
			hit_mpiTestError(okAvgTimes,"Failed ialltoall wait");

			int okRedisTimes = MPI_Wait( &reqRedisTimes, MPI_STATUS_IGNORE );
			hit_mpiTestError(okRedisTimes,"Failed ialltoall wait");

			currentALBIter++;	
			
			dev = 1;

			/* COMPUTE NEW WEIGHTS */
			HitWeights weights;
			float normalizedWeights[hit_NProcs];
			double suma = 0;
			double sumaAvg = 0;
			double peor = 0;
			double redisPeor = 0;

			for (int k=0; k<hit_NProcs; k++) {
				suma += hit_tileElemAt(allTimes, 1, k);
				sumaAvg += hit_tileElemAt(avgTimes, 1, k);
				if(hit_tileElemAt(avgTimes, 1, k) > peor){
					peor = hit_tileElemAt(avgTimes, 1, k);
				}
			} 
			double media = sumaAvg/hit_NProcs;

			int iters = 0;
			int itersPeor = 0;

			if(lastRedisSeconds == -1){
				iters = 0;
			} else if((peor-media) != 0.0)  {
				for (int k=0; k<hit_NProcs; k++) {
					if(hit_tileElemAt(redisTimes, 1, k) > redisPeor){
						redisPeor = hit_tileElemAt(redisTimes, 1, k);
					}
				}
				if(currentALBIter != 1){
					mediaRedisTime = ((mediaRedisTime * (currentALBIter-1))+redisPeor)/(currentALBIter-1);
				}
				iters = mediaRedisTime/(peor-media);
				itersPeor = redisPeor/(peor-media);
				
			}

			nextALB = currentIter + iters;

			printf("&2& %d,%d,%d,%lf,%lf,%lf,%d,%d\n", hit_Rank, currentIter, currentALBIter, mediaRedisTime, peor, media, iters, nextALB);


			for (int k=0; k<hit_NProcs; k++) {
				if (hit_tileElemAt(allTimes, 1, k) == 0.0) normalizedWeights[k] = 0.0;
				else {
					normalizedWeights[k] = (float) (suma / hit_tileElemAt(allTimes, 1, k));
				}
			}
			if(suma == 0.0) normalizedWeights[0] = 1;
			weights = hitWeights( hit_NProcs, normalizedWeights );

			#ifdef DEBUG
				if (hit_layImLeader((layoutComm))){
					printf("Process[%d] weights: ", hit_Rank);
					printArrayFloat(normalizedWeights, hit_NProcs);
					fflush(stdout);
				}
			#endif

			/* MOVE MATRIX TO HOST */
			Ctrl_MoveFrom(comm, tileData->tileMat);
			Ctrl_WaitTile(comm, tileData->tileMat);
			Ctrl_MoveFrom(comm, tileData->tileInnerLocal);
			Ctrl_WaitTile(comm, tileData->tileInnerLocal);

			HitTile *globalMat = hit_tileRoot(&tileData->tileMat);

			/* Macro conservada por si es necesaria para debugging
			#define printRefs(_refs, _dims, _numborders)															\
			{																										\
				printf("tileMatRef %p\n",(void*) _refs.tileMatRef);													\
				printf("tileInnerLocalRef %p\n",(void*) _refs.tileInnerLocalRef);									\
				printf("io_tileRef %p\n",(void*) _refs.io_tileRef);													\
				printf("tileBorderOutDevRefs %p\n",(void*) _refs.tileBorderOutDevRefs);								\
				printf("*tileBorderOutDevRefs %p\n",(void*) *(_refs.tileBorderOutDevRefs));							\
				printf("tileBorderInRefs %p\n",(void*) _refs.tileBorderInRefs);										\
				printf("*tileBorderInRefs %p\n",(void*) *(_refs.tileBorderInRefs));									\
				for(int _i; _i<dims; _i++){																			\
					printf("tileBorderOutDevRefs + %d + 0 %p\n", _i,(void*) *((_refs.tileBorderOutDevRefs+_i*2)+0));\
					printf("tileBorderOutDevRefs + %d + 1 %p\n", _i,(void*) *((_refs.tileBorderOutDevRefs+_i*2)+1));\
				}																									\
				for(int _i = 0; _i<_numborders; _i++){																\
					printf("tileBorderInRefs + %d %p\n", _i,(void*) *((_refs.tileBorderInRefs)+_i));				\
				}																									\
			}
			*/

			TileRefs tileRefs;
			TileRefs copyRefs;


			(tileRefs).tileBorderOutDevRefs = malloc(sizeof(*((tileRefs).tileBorderOutDevRefs)) * dims * 2);
			(copyRefs).tileBorderOutDevRefs = malloc(sizeof(*((tileRefs).tileBorderOutDevRefs)) * dims * 2);
			(tileRefs).tileBorderInRefs = malloc(sizeof(*((copyRefs).tileBorderOutDevRefs)) * (*dataCommon).numBorders);
			(copyRefs).tileBorderInRefs = malloc(sizeof(*((copyRefs).tileBorderOutDevRefs)) * (*dataCommon).numBorders);
			
			//Macro para guardar las referencias en un tipo de datos
			#define getRefs(_tile, _refs, _dims, _numborders)                  							\
			{																							\
				_refs.tileMatRef = _tile.tileMat.ref;													\
				_refs.tileInnerLocalRef = _tile.tileInnerLocal.ref;										\
				_refs.io_tileRef = _tile.io_tile.ref;													\
				for(int _i; _i<dims; _i++){																\
					*((_refs.tileBorderOutDevRefs+_i*2)+0) = (*((_tile.tileBorderOutDev+_i*2)+0)).ref;	\
					*((_refs.tileBorderOutDevRefs+_i*2)+1) = (*((_tile.tileBorderOutDev+_i*2)+1)).ref;	\
				}																						\
				for(int _i = 0; _i<_numborders; _i++){													\
					*((_refs.tileBorderInRefs)+_i) = (*((_tile.tileBorderIn)+_i)).ref;					\
				}																						\
			}

			//Macro para recolocar las referencias una vez terminada la redistribución
			#define setRefs(_tile, _refs, _dims, _numborders)                  							\
			{																							\
				_tile.tileMat.ref = _refs.tileMatRef;													\
				_tile.tileInnerLocal.ref = _refs.tileInnerLocalRef;										\
				_tile.io_tile.ref = _refs.io_tileRef;													\
				for(int _i = 0; _i<dims; _i++){															\
					(*((_tile.tileBorderOutDev+_i*2)+0)).ref = *((_refs.tileBorderOutDevRefs+_i*2)+0);	\
					(*((_tile.tileBorderOutDev+_i*2)+1)).ref = *((_refs.tileBorderOutDevRefs+_i*2)+1);	\
				}																						\
				for(int _i = 0; _i<_numborders; _i++){													\
					(*((_tile.tileBorderIn)+_i)).ref = *((_refs.tileBorderInRefs)+_i);					\
				}																						\
			}

			getRefs((*tileData), tileRefs, dims, ((*dataCommon).numBorders));
			getRefs((*tileDataCopy), copyRefs, dims, ((*dataCommon).numBorders));

			/* FREE OLD TILECOPY */
			Ctrl_Free(comm, tileDataCopy->tileMat);

			/* CREATE NEW LAYOUT */
			HitLayout newLay;
			newLay = hit_layout_freeTopo(plug_layDimWeighted_Blocks, hit_topology(plug_topPlain), lay->origShape, 0, weights);
			HitShape newShape;
			newShape = hit_layShape(newLay);

			//printHitShape2(*shpLayout);
			//printHitShape2(newShape);
			
			/* CREATE NEW TILES */
			TileData newTiles;
			TileData newTilesCopy;

			/* COMPUTE NEW TILE */
			allocateTileData(&newTiles, *dataCommon, dims);
			computeCommonData(dataCommon, shp, newShape, newLay, stencil, dims, stencilData, borderLow, borderHigh);
			computeSingleTileData(&newTiles, dataCommon, comm, shp, newShape, newLay, stencil, dims, stencilData, borderLow, borderHigh, globalMat);
			

			/* 4.8. INITIALIZE ARRAY */
			// FORMATO DEL OUTPUT AQUI
			if (hit_Rank == 0) {
				printf("ALB Redistribution\n");
				fflush(stdout);
			}
			if (hit_Rank == 0) {
				printf("\nPartition weights = {");
				for (int i = 0; i < weights.num_procs; i++)
					printf(" %f,", weights.ratios[i]);
				printf("\b }\n");
				fflush(stdout);
			}

			/* REDISTRIBUTE */
			hit_patternDoOnce(hit_patternLayRedistributeGeneric(*lay, newLay, &(tileData->tileMat), &((newTiles).tileMat), HIT_FLOAT, expandShapeBorders, expandShapeBordersAndHalos));

			/* FREE OLD TILE */
			Ctrl_Free(comm, tileData->tileMat);

			/* COMPUTE NEW TILE COPY */
			allocateTileData(&newTilesCopy, *dataCommon, dims);
			computeSingleTileData(&newTilesCopy, dataCommon, comm, shp, newShape, newLay, stencil, dims, stencilData, borderLow, borderHigh, globalMat);

			// TO-DO Cambiar esto para que solo copie los bordes
			// We copy the data from the tile to the copy so the borders are initialized,
			// without the need to call the function to initialize it
			char *omp_env     = getenv("OMP_NUM_THREADS");
			int   omp_threads = (omp_env != NULL) ? atoi(omp_env) : 1;
			#pragma omp parallel for num_threads(omp_threads)
			for (int i = 0; i < (newTiles).tileMat.acumCard; i++)
				hit((newTilesCopy).tileMat, i) = hit((newTiles).tileMat, i);


			/* COMMUNICATE HALOS */
			hit_patternDo((newTiles).neighSync);
			
			/* UPDATE LAYOUT */

			hit_layFree(*lay);
			hit_shapeFree(*shpLayout);

			*lay = newLay;
			*shpLayout = newShape;

			/* FREE OLD TILE DATA */
			freeTileData(tileData);
			freeTileData(tileDataCopy);
			
			*tileData = newTiles;
			*tileDataCopy = newTilesCopy;
			

			/* CREATE IO TILES */
			selectIOTiles(tileData, tileDataCopy, dataCommon, borderLow, borderHigh);

			setRefs((*tileData), tileRefs, dims, ((*dataCommon).numBorders));
			setRefs((*tileDataCopy), copyRefs, dims, ((*dataCommon).numBorders));


			free((tileRefs).tileBorderOutDevRefs);
			free((copyRefs).tileBorderInRefs);
			
			/* MOVE MATRIX TO DEVICE */
			Ctrl_MoveTo(comm, tileData->tileMat, tileDataCopy->tileMat);
			Ctrl_WaitTile(comm, tileData->tileMat, tileDataCopy->tileMat);
			
			#define copy2Threads( th, arr ) {th.i = arr[0]; th.j = arr[1]; th.k = arr[2];}
			// clang-format on

			int threadsInnerCoords[3] = {1, 1, 1};

			for (int i = 0; i < dims && i < 3; i++) {
				threadsInnerCoords[i] = hit_tileDimCard((newTiles).tileInnerLocal, i);
			}
			copy2Threads((*threadsInner), threadsInnerCoords);

			for (int i = 0; i < dims; i++) {
				int coords[2][3] = {{1, 1, 1}, {1, 1, 1}};
				for (int j = 0; j < dims && j < 3; j++) {
					coords[0][j] = hit_tileDimCard(*(((newTiles).tileBorderOutDev+i*2)+0), j);
					coords[1][j] = hit_tileDimCard(*(((newTiles).tileBorderOutDev+i*2)+1), j);
				}
				copy2Threads((*(((thrBorderOutDev)+i*2)+0)), coords[0]);
				copy2Threads((*(((thrBorderOutDev)+i*2)+1)), coords[1]);
			}

			/* Reset average */
			hit_avgResetData(avg);
			printf("&1& %d,", hit_Rank);
			for (int i = 0; i < weights.num_procs; i++)
				printf(" %f,", weights.ratios[i]);
			printf("\b");

			fflush(stdout);
			hit_clockStop(redisClock);
		}
	}
	currentIter++;	
	hit_clockStart(callClock);
	return dev;
}

HitShape expandShapeBorders(HitTile* globalMat, int* borderLow, int* borderHigh, HitShape shape){
		
	int dims = hit_shapeDims(shape);
	
	for (int i = 0; i < dims; i++) {
		/* DIM i FIRST BORDER IS NOT MINE */
		
		if (hit_sigIn(hit_shapeSig(shape, i), (hit_tileDimBegin(*globalMat, i)+borderLow[i]))) {
			shape = hit_shapeTransform(shape, i, HIT_SHAPE_BEGIN, -borderLow[i]);
		}
		/* DIM i LAST BORDER IS NOT MINE */
		if (hit_sigIn(hit_shapeSig(shape, i), (hit_tileDimEnd(*globalMat, i)-borderHigh[i]))) {
			shape = hit_shapeTransform(shape, i, HIT_SHAPE_END, +borderHigh[i]);
		}
	}
	return shape;
}

HitShape expandShapeBordersAndHalos(HitTile* globalMat, int* borderLow, int* borderHigh, HitShape shape){
		
	int dims = hit_shapeDims(shape);
	
	for (int i = 0; i < dims; i++) {
		/* DIM i FIRST BORDER IS NOT MINE */
		
		shape = hit_shapeTransform(shape, i, HIT_SHAPE_BEGIN, -borderLow[i]);
		/* DIM i LAST BORDER IS NOT MINE */
		shape = hit_shapeTransform(shape, i, HIT_SHAPE_END, +borderHigh[i]);
	}
	return shape;
}



