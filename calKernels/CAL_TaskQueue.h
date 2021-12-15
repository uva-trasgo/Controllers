/*
 * CAL: Task queues inside the controllers
 *
 * @version 2.0
 * @author Arturo Gonzalez-Escrinano
 * @date Sep 2016
 */


#ifndef _CAL_TaskQueue_
#define _CAL_TaskQueue_

#include<stdlib.h>
#include<cuda.h>
#include<cuda_runtime.h>
#include<omp.h>
#include "CAL_KImpl.h"
#include "CAL_KRequest.h"

typedef enum {
	CAL_TASK_TYPE_NULL,
	CAL_TASK_TYPE_KERNEL,
	CAL_TASK_TYPE_DESTROYCNTRL,
	CAL_TASK_TYPE_GLOBALSYNC,
	CAL_TASK_TYPE_ATTACH,
	CAL_TASK_TYPE_DETACH,
	CAL_TASK_TYPE_MOVETO,
	CAL_TASK_TYPE_MOVEFROM,
	CAL_TASK_TYPE_WAITTILE
} CALTaskType;

typedef struct CAL_Task {
        int deviceId;              // The device id inside the Ctrl
		int async:1;			   // Flag for async tasks: Synchronous 0, Asynchronous 1.
        CALTaskType label;         // A task label (Future optimization: Reuse predefined tasks)
        int position;                   // @ana:  Position in the queue. Knowing the number of the kernel and the amount of kernels executed, we know the actual position in the queue
        int narguments;			// Number of arguments/roles/pointers
        int (* kernelWrapper)(cudaStream_t handler, CAL_Impl cntrlS, int impl, CALThread threads, void * arguments );  //MANOLO  // Kernel launching wrapper pointer for CPUs
        void * arguments;       // Packed list of arguments
        char * roles;           // Input/Output roles, for memory optimizations
        void ** pointers;       // Pointers to the original variables, for memory basic operations
        uint8_t * displacements; // Displacement of parameter over arguments array, for memory basic operations
        struct CAL_Task * next; // Next task in the queue
        CALThread threads;      // Index domain where the task is executed
		CAL_Request* outrequest;  // Output handler (Request) of concurrent kernel
		int ninrequests; 			  // Number of elements of inrequests
		CAL_Request* inrequests; // Input handlers (request) current kernel launch has to wait
} CAL_Task;

/* CAL_Task: deviceId value to indicate all the devices in displacementthe controller */
#define CAL_DEVICE_ALL  -1

/* CAL_Task: NULL value */
#define CAL_TASK_NULL   { CPU, 0, CAL_TASK_TYPE_NULL, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL }

/* 
 * Task Queue definition
 */
typedef struct CAL_TaskQueue {
        CAL_Task * head;
        CAL_Task * tail;
} CAL_TaskQueue;


/* 
 * Task Queue functionalities: Init
 */
static inline void CAL_TaskQueue_init( CAL_TaskQueue * q ) {
	q->head = NULL; 
	q->tail = NULL;
}

/* 
 * Task Queue functionalities: Push
 */
static inline void CAL_TaskQueue_push( CAL_TaskQueue * q, CAL_Task task ) {
	CAL_Task *newT = (CAL_Task *)malloc( sizeof( CAL_Task ) );
	if ( newT == NULL ) { fprintf(stderr, "CAL Internal error: Task Malloc\n"); exit(-1); } 
	*newT = task;
	newT->next = NULL;
	if ( q->tail != NULL ) q->tail->next = newT;
	q->tail = newT;
	if ( q->head == NULL ) q->head = newT;


}

//Yuri
//static inline void CAL_TaskQueue_set_stream(CAL_Task *task, cudaStream_t * stream) {		
//	task->stream = stream;
//}

// TODO: created by ismael @ 2017-12-15 14:37:52
// Understand use of stream as parameter
//Yuri añadir una tarea concurrent kernel
static inline void CAL_TaskQueue_push_CKernel( CAL_TaskQueue * q, CAL_Task task ) {
	CAL_Task *newT = (CAL_Task *)malloc( sizeof( CAL_Task ) );
	if ( newT == NULL ) { fprintf(stderr, "CAL Internal error: Task Malloc\n"); exit(-1); } 
	*newT = task;
	// newT->stream = stream;
	// newT->label= 10;
	newT->async = 1;
	newT->next = NULL;
	if ( q->tail != NULL ) q->tail->next = newT;
	q->tail = newT;
	if ( q->head == NULL ) q->head = newT;

}


/* 
 * Task Queue functionalities: Pop
 */
static inline CAL_Task CAL_TaskQueue_pop( CAL_TaskQueue * q ) {
	CAL_Task ret = CAL_TASK_NULL;
	if( q->head != NULL ) { ret = *(q->head); }
	if ( q->tail == q->head ) q->tail = NULL;
	if ( q->head != NULL ){ q->head = q->head->next; }	
	return ret;
}

static inline CAL_Task* CAL_TaskQueue_pointer_pop( CAL_TaskQueue * q ) {
	CAL_Task *pret = NULL;
	if( q->head != NULL ) { pret = q->head; }
	if ( q->tail == q->head ) q->tail = NULL;
	if ( q->head != NULL ){ q->head = q->head->next; }	
	return pret;
}



//Yuri
//static inline cudaStream_t * CAL_TaskQueue_get_stream( CAL_TaskQueue * q ) {	
	
//	cudaStream_t *stream;	
//	stream = q->head->stream;	 
//	return stream;
//}


/* 
 * Task Queue functionalities: Check if it is empty
 */
static inline int CAL_TaskQueue_is_empty( CAL_TaskQueue * q ) {
	return q->head == NULL;
}

/* 
 * Task Queue functionalities: Clean (eliminate tasks)
 */
static inline void CAL_TaskQueue_clean( CAL_TaskQueue * q ) {
	while( q->head != NULL ) {
		CAL_Task * head = q->head;
		q->head = q->head->next;
		free( head );
	}
	q->tail = NULL;
}

/* 
 * Task Queue: DEBUG functionalities for our unit tests
 
static inline void CAL_TaskQueue_debug( CAL_TaskQueue * q ) {
	printf("-- Debug Tasks Start\n");
	CAL_Task *p = q->head;
	CAL_Task *last = NULL;
	
	while( p != NULL ) {
		printf("-- Task: %d\n", p->label );
		last = p;
		p = p->next;
	}
	printf("-- Last equal tail: ");
	if ( last == q->tail ) printf("OK\n");
	else printf("X\n");
	printf("-- Debug Tasks Stop\n");
}
static inline void CAL_TaskQueue_debugConsume( CAL_TaskQueue * q ) {
	printf("-- Debug Tasks Consume Start\n");
	CAL_Task task;
	
	while( ! CAL_TaskQueue_is_empty( q ) ) {
		task = CAL_TaskQueue_pop( q );
		printf("-- Task: %d\n", task.label );
	}
	printf("-- Empty Queue: ");
	if ( q->head == NULL && q->tail == NULL ) printf("OK\n");
	else printf("X\n");
	printf("-- Debug Tasks Consume Stop\n");
}
*/
#endif // _CAL_TaskQueue_
