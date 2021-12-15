#ifndef _TIMER_H_
#define _TIMER_H

#include<sys/time.h>

typedef struct {
	struct timeval	start;
	struct timeval	stop;
	double total;
} Timer;

#define	TimerCreate(timer)	timer.total = 0.0
#define TimerDestroy(timer)
#define TimerContinue(timer)	gettimeofday( &(timer.start), 0 )
#define TimerStart(timer)	timer.total = 0.0; TimerContinue(timer)
#define TimerStop(timer)	gettimeofday( &(timer.stop), 0 ); timer.total += \
					(timer.stop.tv_sec - timer.start.tv_sec) + \
					0.000001 * (timer.stop.tv_usec - timer.start.tv_usec)
#define TimerGetTime(timer)	timer.total

#endif
