#ifndef CAL_CNTRLCPU_H
#define CAL_CNTRLCPU_H


#include "CAL_Thread.h"

#include <hitmap.h>
#include <omp.h>
#include "CAL_KProto.h"

#define MAX_TILES 256
#define MIN_TASK 3000





/*  * @brief Information about the status of a specific CPU controller.  */ 
typedef struct {         
	//number of task in the queue 	
	int ntask;         
	//CPUs for this controller         
	int CPUS; 	
	omp_lock_t destroyCntrl; // Locks to control the controller 	
	omp_lock_t lockQueue;         
	// @ana 24-07-2017: Integrating arturo's queue          
	CAL_TaskQueue listTask; 
} CALCntrlCPU;


#ifdef __cplusplus
extern "C" {
#endif

// Define functions headers
void CAL_CntrlCPUCreate(CALCntrlCPU* cntrl, int num_cpus);
void CAL_CntrlCPUDestroy(CALCntrlCPU* citrl);
void CAL_CntrlCPUAttach(CALCntrlCPU* citrl, HitTile* tile);
void CAL_CntrlCPUDetach(CALCntrlCPU* cntrl, HitTile* tile);
void CAL_CntrlCPUCreateInternal(CALCntrlCPU* cntrl, HitTile* tile);
void CAL_CntrlCPUAddTask(CALCntrlCPU* cntrl, void (*func)(int,void**), CALThread threads, int argc, ...);



#ifdef __cplusplus
}
#endif

#endif /* CAL_CNTRL_CPU_H */
