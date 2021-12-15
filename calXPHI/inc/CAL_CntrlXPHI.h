#ifndef CAL_CNTRLXPHI_H
#define CAL_CNTRLXPHI_H

#include "CAL_Thread.h"

#include <hitmap.h>
#include <omp.h>
#include "CAL_KProto.h"
#include "utilities.h"


#define MAX_TILES 256
#define MIN_TASK 3000



/*
 * @brief Information about the status of a specific XPHI controller.
 */
typedef struct {
        //number of task in the queue
        int ntask;
        //XPHIs for this controller
        int XPHIS;
        // TODO use the new Arturo queue
	omp_lock_t destroyCntrl; // Locks to control the controller
        omp_lock_t lockQueue;
	CAL_TaskQueue listTask;
} CALCntrlXPHI;


#ifdef __cplusplus
extern "C" {
#endif

// Define functions headers
void CAL_CntrlXPHICreate(CALCntrlXPHI* cntrl, int num_cpus);
void CAL_CntrlXPHIDestroy(CALCntrlXPHI* cntrl);
void CAL_CntrlXPHIAttach(CALCntrlXPHI* cntrl, HitTile* tile);
void CAL_CntrlXPHIDetach(CALCntrlXPHI* cntrl, HitTile* tile);
void CAL_CntrlXPHICreateInternal(CALCntrlXPHI* cntrl, HitTile* tile);
void CAL_CntrlXPHIAddTask(CALCntrlXPHI* cntrl, void (*func)(int,void**), CALThread threads, int argc, ...);

#ifdef __cplusplus
}
#endif
/* NEW */
//int hit_tileNumElem(HitTile *m);
void attachingToXPHI(CALCntrlXPHI* cntrl, HitTile *tile);
static inline void detachingToXPHI(CALCntrlXPHI* cntrl, HitTile *tile);

#endif /* CAL_CNTRL_XPHI_H */
