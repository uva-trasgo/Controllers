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

#include "hit_automatic_load_balancing/hit_alb.h"
#include "Ctrl_Core.h"
#include <math.h>

hit_tileNewType(double);
Ctrl_NewType(float);

typedef struct tileData {
	HitTile_float tileMat;
	HitTile_float tileInnerLocal;
	HitTile_float io_tile;
	HitTile_float *tileBorderOutDev;
	HitTile_float *tileBorderIn;
	HitPattern neighSync;
} TileData;
typedef struct tileCommon {
	int num_tiles_total;
	int numBorders;
	int *borderInActive;
	int dims;
} TileCommon;
typedef struct tileRefs {
	HitTile *tileMatRef;
	HitTile *tileInnerLocalRef;
	HitTile *io_tileRef;
	HitTile* (*tileBorderOutDevRefs);
	HitTile* (*tileBorderInRefs);    
} TileRefs;

void allocateTileData(TileData *data, TileCommon common, int dims);
void freeTileData(TileData *data);
void selectIOTiles(TileData *tileData, TileData *tileDataCopy, TileCommon *dataCommon, int* borderLow, int* borderHigh);
void computeCommonData(TileCommon *dataCommon, HitShape shp, HitShape shpLayout, HitLayout lay, HitTile_float stencil, int dims, float stencilData[], int *borderLow, int *borderHigh);
void computeSingleTileData(TileData *tileData, TileCommon *dataCommon, PCtrl comm, HitShape shp, HitShape shpLayout, HitLayout lay, HitTile_float stencil, int dims, float stencilData[], int *borderLow, int *borderHigh, HitTile_float *globalMat);
int Ctrl_ALB(TileData *tileData, TileData *tileDataCopy, TileCommon *dataCommon, HitLayout *lay, HitShape *shpLayout, HitAvg * avg, double time,
		PCtrl comm, HitShape shp, HitTile_float stencil, int dims, float stencilData[], int *borderLow, int *borderHigh,
		Ctrl_Thread *threadsInner, Ctrl_Thread *thrBorderOutDev);

HitShape expandShapeBorders(HitTile* globalMat, int* borderLow, int* borderHigh, HitShape shape);
HitShape expandShapeBordersAndHalos(HitTile* globalMat, int* borderLow, int* borderHigh, HitShape shape);