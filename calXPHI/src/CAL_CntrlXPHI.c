#define _BSD_SOURCE

#include "CAL_CntrlXPHI.h"

#define MIC_DEV 0
#include <assert.h>
#include <omp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define ALLOC   alloc_if(1)
#define FREE    free_if(1)
#define RETAIN  free_if(0)
#define REUSE   alloc_if(0)


/*
 * Create the controller and its corresponding variables
 * Parasm:
 *      cntrl: Controller to be created.
 *      num_cpus: Number of CPUS used by this controller
 */
void CAL_CntrlXPHICreate(CALCntrlXPHI* cntrl, int num_cpus){

        /* Allocate the controller */
        CAL_TaskQueue_init( &(cntrl->listTask) );
        CAL_Task task = CAL_TASK_NULL;

        /* Initialize the data structure */
        cntrl->ntask = 0;
        omp_init_lock(&(cntrl->destroyCntrl));
        omp_init_lock(&(cntrl->lockQueue));
        // Blocking controller  
        omp_set_lock(&(cntrl->destroyCntrl));

#ifdef DEBUG
        printf("Controller created with %d CPUS.\n", cntrl->CPUS);
        fflush(stdout);
#endif

        #pragma omp task 
        {
        {
        {
        #ifdef DEBUG
        printf("\n CPUS:%d", omp_get_num_threads());
        #endif
        int taskDone=0;
        int FlagFinish=0;

        // TODO improve the active wait using semaphores
        // While the controller has not been destroyed
        while( FlagFinish==0){

            #ifdef DEBUG
              printf("\n Waiting task taskDone:%d. \n", taskDone);
            #endif

           // If there are tasks in the queue
           #pragma omp flush(cntrl)
           if(! CAL_TaskQueue_is_empty( &(cntrl->listTask) )) {

                // Take the first task --> an improvement can be to make a better choice (not the first in the queue)
                omp_set_lock(&(cntrl->lockQueue));
                CAL_Task run = CAL_TaskQueue_pop(&(cntrl->listTask));
                omp_unset_lock(&(cntrl->lockQueue));


                 #ifdef DEBUG
                   printf("\n Task chosen: %d \n", (run).label);
                 #endif


                 // Task executing a Kernel
                 if((run).label == CAL_TASK_TYPE_KERNEL){

                        #ifdef DEBUG
                                printf("\n Executing work. \n");
                        #endif

                        // Execute task
			//run.kernelWrapper( XPhi,run.deviceId, run.threads,  run.arguments );			//yuri
                        // Counting the number of executed kernels
                        taskDone++;

                        // Free the executed task
                        free(run.roles);
                        free(run.pointers);
                        free(run.arguments);
                        #ifdef DEBUG
                         printf("\n Executed work. \n");
                        #endif
                        }

               // Last task for destroying the Controller
                if((run).label == CAL_TASK_TYPE_DESTROYCNTRL){
                        #ifdef DEBUG
                                printf("Unlocking.\n");
                        #endif
                        FlagFinish=1;
                }

                // TODO
                // Task for syncronizing
                // Task for attaching a tile
            	if((run).label == CAL_TASK_TYPE_ATTACH){
                	#ifdef DEBUG
                	printf("Attaching.\n");
                	#endif
			HitTile *tmp = (HitTile*)(run.arguments);
                	attachingToXPHI((cntrl), tmp);
                	taskDone++;
            	}
                // Task for detaching a tile
            	if((run).label == CAL_TASK_TYPE_DETACH){
            	    	#ifdef DEBUG
            	    	printf("Detaching.\n");
            	  	#endif
		    	HitTile *tmp = (HitTile*)(run.arguments);
            	    	detachingToXPHI((cntrl), tmp);
            	    	taskDone++;
            	}

            }
          }
       }
     }
#ifdef DEBUG
  printf("Destroyed\n");
#endif

  // Once the controller is detroyed, clean the queue and unset locks
  omp_set_lock(&(cntrl->lockQueue));
  CAL_TaskQueue_clean(&(cntrl->listTask));
  omp_unset_lock(&(cntrl->lockQueue));
  omp_unset_lock(&(cntrl->destroyCntrl));

   }

}



void CAL_CntrlXPHIDestroy(CALCntrlXPHI* cntrl){

        #pragma omp flush(cntrl)
        int num_task=  (cntrl->ntask);

#ifdef DEBUG
        printf("Eliminating CPU controller. Task %d \n", (cntrl->ntask));
#endif
        // This function is executed by the master thread
        // The controller reachs the end, no more task would be added
        // Add a task with a value -2 in the taskEnd field
        // Allocating new task
        CAL_Task newTask = CAL_TASK_NULL;
        newTask.label= CAL_TASK_TYPE_DESTROYCNTRL;
        newTask.position= num_task;

         //Add task to the cntrl
        CAL_TaskQueue_push( &(cntrl->listTask),newTask );
        cntrl->ntask++;


        //Wait for the syncronization and destroy locks
        omp_set_lock(&(cntrl->destroyCntrl));
        omp_destroy_lock(&(cntrl->destroyCntrl));
        omp_destroy_lock(&(cntrl->lockQueue));

}




void CAL_CntrlXPHIAttach(CALCntrlXPHI* cntrl, HitTile* tile){

        int num_task=  (cntrl->ntask);
        
	// Allocating new task
        CAL_Task newTask = CAL_TASK_NULL;
        newTask.label= CAL_TASK_TYPE_ATTACH;
        newTask.position= num_task;
        newTask.arguments = (HitTile*)tile;
         //Add task to the cntrl
        omp_set_lock(&(cntrl->lockQueue));
        CAL_TaskQueue_push( &(cntrl->listTask),newTask );
        cntrl->ntask++;
        omp_unset_lock(&(cntrl->lockQueue));
}

/*
 * Dettach tiles in a controller
 */
void CAL_CntrlXPHIDetach(CALCntrlXPHI* cntrl, HitTile* tile){
        

        int num_task=  (cntrl->ntask);
        
	// Allocating new task
        CAL_Task newTask = CAL_TASK_NULL;
        newTask.label= CAL_TASK_TYPE_DETACH;
        newTask.position= num_task;
        newTask.arguments = (HitTile*)tile;
         //Add task to the cntrl
        omp_set_lock(&(cntrl->lockQueue));
        CAL_TaskQueue_push( &(cntrl->listTask),newTask );
        cntrl->ntask++;
        omp_unset_lock(&(cntrl->lockQueue));
}



void attachingToXPHI(CALCntrlXPHI* cntrl, HitTile *tile){

#if __ICC
    HitTile *tmp=(HitTile*) tile;
    int numElems;
    float *data = (float*) (*tmp).data;
    numElems= hit_tileNumElem((HitTile*)tmp); 

    //char *data = (char*) (*tmp).data;
    //numElems= hit_tileNumElem((HitTile*)tmp* tile->baseExtent);
    if(numElems > 0){
    	#pragma offload target(mic:0)  in(data:length(numElems) align(64) ALLOC RETAIN) //signal(data)
   	 {
		#ifdef DEBUG
		char michostname[100];
 	 	gethostname(michostname, sizeof(michostname));
  		printf("Attaching %p. I am %s and I have %ld logical cores. \n",data, michostname, sysconf(_SC_NPROCESSORS_ONLN));
		#endif
	}
     }
#endif

}
void detachingToXPHI(CALCntrlXPHI* cntrl, HitTile *tile){

#if __ICC
    	HitTile *tmp=(HitTile*) tile;
        int numElems;
        float *data = (float*) (*tmp).data;
        numElems= hit_tileNumElem((HitTile*)tmp);
        //char *data = (char*) (*tile).data;
        //numElems= hit_tileNumElem((HitTile*)tile * tile->baseExtent);
        if(numElems > 0){
           #pragma offload target(mic:0) in(data:length(0) REUSE RETAIN) out(data:length(numElems) REUSE FREE) //signal(data)
           {
		#ifdef DEBUG
		char michostname[100];
 		gethostname(michostname, sizeof(michostname));
  		printf("Detaching %p. I am %s and I have %ld logical cores. \n", data, michostname, sysconf(_SC_NPROCESSORS_ONLN));
		#endif
           }
        }

#endif

}







