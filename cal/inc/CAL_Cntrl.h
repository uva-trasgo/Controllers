#ifndef CAL_CNTRL_H
#define CAL_CNTRL_H

#include <hitmap.h>
#include <omp.h>

#include "CAL_CntrlCPU.h"
#include "CAL_CntrlGPU.h"
#include "CAL_CntrlXPHI.h"
#include "CAL_KRequest.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Tipos de dispositivos que pueden gestionar los controladores.
 */
typedef enum {
        CAL_CNTRL_TYPE_CPU,
        CAL_CNTRL_TYPE_GPU,
        CAL_CNTRL_TYPE_XPHI,

} CALCntrlType ;


// Macros para simular valores por defecto
#define CAL_CNTRL_CPU CAL_CNTRL_TYPE_CPU, 0
#define CAL_CNTRL_GPU CAL_CNTRL_TYPE_GPU, 1
#define CAL_CNTRL_XPHI CAL_CNTRL_TYPE_XPHI, 0

/**
 * @brief Contiene los datos de las estructuras.
 * Se utiliza una unión por eficiencia.
 */
typedef union {
        CALCntrlCPU cpu;
        CALCntrlGPU gpu;
        CALCntrlXPHI xphi;
} CALCntrlDevice;

/**
 * @brief Controlador abstracto.
 */
typedef struct {
        CALCntrlType    type;           /**< Tipo de dispositivo que gestiona el controlador. */
        CALCntrlDevice  impl;           /**< Contiene las estructuras para los datos concretos para cada tipo de dispositivo. */
        int device;                     /**< Número para indicar el dispositivo concreto dentro de los del mismo tipo. */
} CALCntrl;


#define MAX_TILES 256

#define CAL_CntrlSync(cntrl ) \
	switch (cntrl.type) \
	{ \
		case CAL_CNTRL_TYPE_GPU: \
			CAL_Task newTask = CAL_TASK_NULL;			\
			newTask.label=CAL_TASK_TYPE_GLOBALSYNC;					\
        		newTask.position= (cntrl.impl.gpu.ntask);			\
				omp_set_lock(&(cntrl.impl.gpu.lockQueue)); \
					CAL_TaskQueue_push( &(cntrl.impl.gpu.listTask), newTask ); \
					cntrl.impl.gpu.ntask++; \
                omp_unset_lock(&(cntrl.impl.gpu.lockQueue)); \
			/* Block the host, waiting */			\
			omp_set_lock(&(cntrl.impl.gpu.syncGlobalCntrl)); 		\
				newTask.label=CAL_TASK_TYPE_GLOBALSYNC;					\
        			newTask.position= (cntrl.impl.gpu.ntask);			\
				omp_set_lock(&(cntrl.impl.gpu.lockQueue)); \
					CAL_TaskQueue_push( &(cntrl.impl.gpu.listTask), newTask ); \
					cntrl.impl.gpu.ntask++; \
                omp_unset_lock(&(cntrl.impl.gpu.lockQueue)); \
			omp_set_lock(&(cntrl.impl.gpu.syncGlobalCntrl)); 		\
	 }



//omp_set_lock(&(cntrl.impl.gpu.syncGlobalCntrl));

#define CAL_CntrlLaunch(cntrl, name, threads, ...) \
	switch (cntrl.type) \
	{ \
		case CAL_CNTRL_TYPE_CPU: \
				omp_set_lock(&(cntrl.impl.cpu.lockQueue)); \
    				CAL_TaskQueue_push( &(cntrl.impl.cpu.listTask), CAL_kernelTaskCreate_##name( CPU,threads, CAL_ARGS_TO_POINTERS( __VA_ARGS__) ) ); \
    				cntrl.impl.cpu.ntask++; \
				omp_unset_lock(&(cntrl.impl.cpu.lockQueue)); \
		break; \
		case CAL_CNTRL_TYPE_GPU: \
			cudaSetDevice(cntrl.impl.gpu.numGPU); \
			cudaDeviceSynchronize(); \
			/* Device pointers to the task */						\
			omp_set_lock(&(cntrl.impl.gpu.lockQueue)); \
    			CAL_TaskQueue_push( &(cntrl.impl.gpu.listTask), CAL_kernelTaskCreate_##name( dGPU,threads, CAL_ARGS_TO_POINTERS( __VA_ARGS__) ) ); \
    			cntrl.impl.gpu.ntask++; \
			omp_unset_lock(&(cntrl.impl.gpu.lockQueue)); \
		break; \
		case CAL_CNTRL_TYPE_XPHI: \
				omp_set_lock(&(cntrl.impl.xphi.lockQueue)); \
    				CAL_TaskQueue_push( &(cntrl.impl.xphi.listTask), CAL_kernelTaskCreate_##name( XPhi,threads, CAL_ARGS_TO_POINTERS( __VA_ARGS__) ) ); \
    				cntrl.impl.xphi.ntask++; \
				omp_unset_lock(&(cntrl.impl.xphi.lockQueue)); \
		break; \
		default: printf("Error no controller implementation\n");	\
	}

// TODO: created by ismael @ 2017-12-15 14:10:17
// Mask stream's use
//yuri
#define _CAL_CntrlLaunchCKernel(cntrl, name, threads, outrequest, count, inrequests, ...) {\
	switch (cntrl.type) \
	{ \
		case CAL_CNTRL_TYPE_CPU: \
				omp_set_lock(&(cntrl.impl.cpu.lockQueue)); \
    				CAL_TaskQueue_push( &(cntrl.impl.cpu.listTask), CAL_kernelTaskCreate_##name( CPU,threads, CAL_ARGS_TO_POINTERS( __VA_ARGS__) ) ); \
    				cntrl.impl.cpu.ntask++; \
				omp_unset_lock(&(cntrl.impl.cpu.lockQueue)); \
		break; \
		case CAL_CNTRL_TYPE_GPU: \
			cudaSetDevice(cntrl.impl.gpu.numGPU); \
			/* Device pointers to the task */						\
			omp_set_lock(&(cntrl.impl.gpu.lockQueue)); \
    			/* CAL_TaskQueue_push_CKernel( &(cntrl.impl.gpu.listTask), CAL_concurrentKernelTaskCreate_##name( dGPU, threads, outrequest, count, inrequests, __VA_ARGS__ ) ); */ \
    			CAL_TaskQueue_push( &(cntrl.impl.gpu.listTask), CAL_concurrentKernelTaskCreate_##name( dGPU, threads, outrequest, count, inrequests, CAL_ARGS_TO_POINTERS( __VA_ARGS__) ) ); \
    			cntrl.impl.gpu.ntask++; \
			omp_unset_lock(&(cntrl.impl.gpu.lockQueue)); \
		break; \
		case CAL_CNTRL_TYPE_XPHI: \
				omp_set_lock(&(cntrl.impl.xphi.lockQueue)); \
    				CAL_TaskQueue_push( &(cntrl.impl.xphi.listTask), CAL_kernelTaskCreate_##name( XPhi,threads, CAL_ARGS_TO_POINTERS(   __VA_ARGS__) ) ); \
    				cntrl.impl.xphi.ntask++; \
				omp_unset_lock(&(cntrl.impl.xphi.lockQueue)); \
		break; \
		default: printf("Error no controller implementation\n");	\
	}\
}

#define CAL_CntrlLaunchCKernel(cntrl, name, threads, request, ...)\
	_CAL_CntrlLaunchCKernel(cntrl, name, threads, request, 0, CAL_REQUEST_NULL, __VA_ARGS__)

#define CAL_CntrlLaunchCKernelWait(cntrl, name, threads, outrequest, inrequest, ...)\
	_CAL_CntrlLaunchCKernel(cntrl, name, threads, outrequest, 1, inrequest, __VA_ARGS__)

#define CAL_CntrlLaunchCKernelWaitAll(cntrl, name, threads, outrequest, count, inrequests, ...)\
	_CAL_CntrlLaunchCKernel(cntrl, name, threads, outrequest, count, inrequests, __VA_ARGS__)


#define CAL_CntrlInit(numCntrlCPU) \
	omp_set_nested(1); \
	omp_set_num_threads(numCntrlCPU+1); \
	_Pragma(" omp parallel "  ){ \
	_Pragma("omp single ") {

#define CAL_CntrlFinish() }}



/**
 * @brief Crea un controlador.
 *
 * @param[out]		cntrl	Controlador a crear.
 * @param[in]		type	Tipo de dispositivo del controlador.
 * @param[in]		device	Dispositivo al que se le asigna el controlador.
 */
void CAL_CntrlCreate(CALCntrl* cntrl, CALCntrlType type, int device, int num_cpus);

/**
 * @brief Libera los recursos utilizados por un controlador.
 *
 * @param[in]		cntrl	Controlador que se va a eliminar.
 */
void CAL_CntrlDestroy(CALCntrl* cntrl);

//Yuri (sincronización global con el host)
void CAL_CntrlGlobalSync(CALCntrl* cntrl);

/**
 * @brief Enlaza un tile al controlador.
 *
 * @param[in,out]	cntrl	Controlador con el que se enlaza.
 * @param[in]		tile	Tile a enlazar.
 */
void CAL_CntrlAttach(CALCntrl* cntrl, HitTile* tile);

// TODO: created by ismael @ 2017-12-15 14:11:07
// Mask stream's use
//Yuri
void CAL_CntrlAttachAsyn(CALCntrl* cntrl, HitTile* tile);

//Yuri
void CAL_CntrlMoveTo(CALCntrl* cntrl, HitTile* tile);

// TODO: created by ismael @ 2017-12-15 14:11:38
// Mask stream's use
//Yuri
void CAL_CntrlMoveToAsyn(CALCntrl* cntrl, HitTile* tile);


//Yuri
//cudaStream_t *  CAL_CntrlGetStream(CALCntrl* cntrl);

// TODO: created by ismael @ 2017-12-15 14:11:48
// Mask stream's use
//Yuri
void  CAL_CntrlWaitStream(CALCntrl* cntrl, cudaStream_t stream);

/**
 * @brief Desenlaza un tile del controlador.
 *
 * @param[in,out]	cntrl 	Controlador con el que se desenlaza.
 * @param[in,out]	tile	Tile a desenlazar.
 */
void CAL_CntrlDetach(CALCntrl* cntrl, HitTile* tile);

// TODO: created by ismael @ 2017-12-15 14:12:04
// Mask stream's use
//Yuri
void CAL_CntrlDetachAsyn(CALCntrl* cntrl, HitTile* tile);

//Yuri
void CAL_CntrlMoveFrom(CALCntrl* cntrl, HitTile* tile);

// TODO: created by ismael @ 2017-12-15 14:12:18
// Mask stream's use
//Yuri
void CAL_CntrlMoveFromAsyn(CALCntrl* cntrl, HitTile* tile);

// TODO: created by ismael @ 2017-12-15 14:15:29
// Implement async version
/*
 * @brief Crea un tile interno del controlador.
 *
 * @param[in,out]	cntrl	Controlador en el que se crea el tile.
 * @param[in]		tile	Tile para uso interno. Tiene que tener definido el dominio.
 */
void CAL_CntrlInternal(CALCntrl* cntrl, HitTile* tile);

// TODO: created by ismael @ 2017-12-15 14:15:44
// Implement async version
/*
 * @brief Elimina un tile interno del controlador.
 *
 * @param[in,out]	cntrl	Controlador en el que se crea el tile.
 * @param[in]		tile	Tile para uso interno a eliminar.
 * @param[i]		int	1 for recovering data, 0 only to free memory
 */
void CAL_CntrlDestroyInternal(CALCntrl* cntrl, HitTile* tile, int recover);

/*
 * Function to obtain the best kernel implementation
 */
int CAL_dev(CAL_Impl cntrl, int sumImpl);

/*
 * @brief Initialize struct of CAL_Request
 */
void CAL_CntrlRequestCreate(CALCntrl* cntrl, CAL_Request** request);

/*
 * @brief Destroy struct of CAL_Request
 */
void CAL_CntrlRequestDestroy(CALCntrl* cntrl, CAL_Request* request);

#ifdef __cplusplus
}
#endif

#endif /* CAL_CNTRL_H */
