#ifndef _TIMER_H_
#define _TIMER_H

#if defined (__NVCC__)
	//#warning "Using NVCC Timer"	
#elif defined (_OPENMP)
	//#warning "Using OPENMP Timer"	
	#include <omp.h>
#endif

typedef struct {
#if defined (__NVCC__)
	cudaEvent_t start;
	cudaEvent_t stop;
	float elapsed;
#elif defined (_OPENMP)
	double initTime;
#endif
	double totalTime;
} Timer;
#endif

#if defined (__NVCC__)
	#define TimerCreate(timer) cudaEventCreate(&timer.start); cudaEventCreate(&timer.stop)
	#define TimerDestroy(timer) cudaEventDestroy(timer.start); cudaEventDestroy(timer.stop)
	#define TimerContinue(timer) cudaEventRecord(timer.start)
	#define TimerStop(timer) cudaEventRecord(timer.stop); cudaEventSynchronize(timer.stop); cudaEventElapsedTime(&timer.elapsed, timer.start, timer.stop); timer.totalTime += timer.elapsed / 1000
#elif defined (_OPENMP) 
	#define TimerCreate(timer)
	#define TimerDestroy(timer)
	#define TimerContinue(timer) timer.initTime = omp_get_wtime()
	#define TimerStop(timer) timer.totalTime += omp_get_wtime() - timer.initTime
#endif

#define TimerStart(timer) timer.totalTime = 0; TimerContinue(timer)
#define TimerGetTime(timer) timer.totalTime 
