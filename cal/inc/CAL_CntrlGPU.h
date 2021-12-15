#ifndef CAL_COMMGPU_H
#define CAL_COMMGPU_H

#include "CAL_Thread.h"

#include <cuda_runtime.h>
#include <cuda.h>
#include <hitmap.h>
#include <omp.h>
#include "CAL_KProto.h"
#include "CAL_KRequest.h"
#include <semaphore.h>

#define MAX_TILES 256
#define MAX_SIZE 256

// #ifndef DEBUG
// #define DEBUG
// #endif //DEBUG

typedef struct {
        HitTile* host;
        HitTile* dev;           /**< Tile en la memoria del dispositivo.                **/
        void * data;            /**< Datos del tile en el dispositivo.                  **/
        int transferred;         /**< Indica si los datos han sido transferidos. **/
        int recover;            /**< Indica si hay que recuperar los datos.             **/
        //cudaStream_t stream;	//Yuri Stream para el tile (comunicaciones asíncronas)
} CALTileGPU;

/*
 * @brief Contiene el estado de un comunicador específico de GPU.
 */

typedef struct {
        int             nAttachedTiles;         /**< Número de tiles enlazados al comunicador.  */
        CALTileGPU*     attachedTiles;          /**< Tabla de mapeo de los tiles enlazados.             */
        int             nInternalTiles;         /**< Número de tiles internos del comunicador.  */
        CALTileGPU*     internalTiles;          /**< Tabla de mapeo del os tiles internos.              */
        int numGPU;                             /**< Número de GPU asignada.*/

        // Create CUDA Stream   
        cudaStream_t streamGPU;                 /**< Número de GPU asignada.*/

        int ntask;
        omp_lock_t destroyCntrl; // Locks to control the controller
        omp_lock_t lockQueue;
        omp_lock_t syncGlobalCntrl;
        // @ana 24-07-2017: Integrating arturo's queue
        CAL_TaskQueue listTask;                 /**< Cola de tareas pendientes.*/
        /* Asynchronoyus mutex controls */
        int isAsync;                            /**< Flag de activación de modelo asíncrono de Controller.*/
        sem_t semHost;                          /**< Semáforo para control de ejecución de hilo principal.*/
        sem_t semCntrl;                         /**< Semáforo para control de ejecución de hilo de la instancia de Controller.*/
} CALCntrlGPU;


#ifdef __cplusplus
extern "C" {
#endif
/*
 * @brief Crea un comunicador de GPU.
 * @param comm Comunicador que se va a crear.
 */
void CAL_CntrlGPUCreate(CALCntrlGPU* comm, int device);
void CAL_CntrlGPUDestroy(CALCntrlGPU* comm);
void CAL_CntrlGPUGlobalSync(CALCntrlGPU* comm);	
/*
 * @brief Recupera stream de CUDA asignado a comunicaciones de un hitTile.
 * @param comm Comunicador utilizado en la comunicaciones. (Uso temporal como parámetro)
 * @param tile HitTile del que se quiere recuperar el stream.
 * @param pstream Puntero a stream donde se va a almacenar.
 */
void CAL_CntrlGPUGetOrCreateTileHandler(CALCntrlGPU* cntrl, HitTile* tile, CAL_Request** pstream);
/*
 * @brief Destruye el stream de CUDA asignado a comunicaciones de un hitTile.
 * @param comm Comunicador utilizado en la comunicaciones. (Uso temporal como parámetro)
 * @param tile HitTile del que se quiere recuperar el stream.
 */
void CAL_CntrlGPUDestroyTileHandler(CALCntrlGPU* cntrl, HitTile* tile);											
void CAL_CntrlGPUAttach(CALCntrlGPU* comm, HitTile* tile);
// TODO: created by ismael @ 2017-12-15 14:13:13
// Mask stream's use
void CAL_CntrlGPUAttachAsyn(CALCntrlGPU* comm, HitTile* tile);			//Yuri
void CAL_CntrlGPUMoveTo(CALCntrlGPU* comm, HitTile* tile);									//Yuri
// TODO: created by ismael @ 2017-12-15 14:13:37
// Mask stream's use
void CAL_CntrlGPUMoveToAsyn(CALCntrlGPU* comm, HitTile* tile);			//Yuri
void CAL_CntrlGPUDetach(CALCntrlGPU* comm, HitTile* tile);
// TODO: created by ismael @ 2017-12-15 14:13:55
// Mask stream's use
void CAL_CntrlGPUDetachAsyn(CALCntrlGPU* comm, HitTile* tile);			//Yuri
void CAL_CntrlGPUMoveFrom(CALCntrlGPU* comm, HitTile* tile);								//Yuri
// TODO: created by ismael @ 2017-12-15 14:14:11
// Mask stream's use
void CAL_CntrlGPUMoveFromAsyn(CALCntrlGPU* comm, HitTile* tile);		//Yuri	
// TODO: created by ismael @ 2017-12-15 14:14:29
// Implement async version
void CAL_CntrlGPUInternal(CALCntrlGPU* comm, HitTile* tile);
// TODO: created by ismael @ 2017-12-15 14:14:48
// Implement async version
void CAL_CntrlGPUDestroyInternal(CALCntrlGPU* comm, HitTile* tile, int recover);
CALTileGPU* CAL_CntrlGetTile(CALCntrlGPU* com, HitTile* tile);
//yuri
//cudaStream_t *  CAL_CntrlGPUGetStream(CALCntrlGPU* comm);				//Yuri
void CAL_CntrlGPUWaitTile(CALCntrlGPU* comm, HitTile* tile);		//Yuri

#ifdef __cplusplus
}
#endif

#endif /* CAL_COMM_GPU_H */
