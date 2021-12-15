#ifndef _TIMER_H_
#define _TIMER_H

#include <omp.h>

typedef struct {
	double initTime;
	double totalTime;
} Timer;
#endif

#define TimerCreate(timer)
#define TimerDestroy(timer)
#define TimerContinue(timer) timer.initTime = omp_get_wtime()
#define TimerStop(timer) timer.totalTime += omp_get_wtime() - timer.initTime
#define TimerStart(timer) timer.totalTime = 0; TimerContinue(timer)
#define TimerGetTime(timer) timer.totalTime 
