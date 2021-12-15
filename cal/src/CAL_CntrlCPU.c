/*
 * @ana: Integrating new queues in the Controller library 
 * 28-07-2017
 */

#include "CAL_CntrlCPU.h"

#include <assert.h>
#include <omp.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))

/*
 * Function to group threads into coarse-grained OpenMP tasks
 * Params:
 * 	run: CAL_Task object with the info about the kernel to be executed
 * 	num_cpus: number of threads associated to the controller object
 * 	n_th: task identifier
 */
void CAL_wrapper_CPU(CAL_Task run, int num_cpus, int n_th){
	 CALThread threads= run.threads; 
#ifdef DEBUG 
         printf("Launching kernel \n"); 
#endif        
	{ 
           {
            int i=0,j=0,k=0; 
            CALThread threadId;
	    // Calculating the part of th domain that computes the n_th task 
            int stripSize =(int) ceil((double) threads.x/((num_cpus)) ); 
            int first= n_th*(stripSize);   
            int last= MIN(first + (stripSize) - 1,(threads.x)-1); 
	
	    // Executing in parallel the 3D threads
            if (threads.z > 1){ 
             for(i=first; i<=last; i++){  
                  for(j=0; j<threads.y; j++){           
                       for(k=0; k<threads.z; k++){            
                             threadId.x = i; 
                             threadId.y = j; 
                             threadId.z = k;
			     // Executing the Kernel function, that was stored in the CAL_Task object 
  	              	     //run.kernelWrapper(CPU, run.deviceId, threadId,  run.arguments ); //yuri
             } } }
            } 
	   // Executing in parallel the 2D threads
           else if (threads.y > 1){
             for(i=first; i<=last; i++){  
                for(j=0; j<threads.y; j++){           
                      threadId.x = i; 
                      threadId.y = j; 
                      threadId.z = 0; 
		      // Executing the Kernel function, that was stored in the CAL_Task object 
  	             // run.kernelWrapper(CPU, run.deviceId, threadId,  run.arguments );    //yuri
             } } 
           }
	   // Executing in parallel the 1D threads
           else {
             {
             for(i=first; i<=last; i++){  
                      threadId.x = i; 
                      threadId.y = 0; 
                      threadId.z = 0; 
		      // Executing the Kernel function, that was stored in the CAL_Task object 
  	              //run.kernelWrapper( CPU,run.deviceId, threadId,  run.arguments ); //yuri
             } 
           }
	  } 
      }
   }
}

/*
 * Create the controller and its corresponding variables
 * Parasm:
 * 	cntrl: Controller to be created.
 * 	num_cpus: Number of CPUS used by this controller
 */
void CAL_CntrlCPUCreate(CALCntrlCPU* cntrl, int num_cpus){
	
	/* Allocate the controller */
	CAL_TaskQueue_init( &(cntrl->listTask) );
        CAL_Task task = CAL_TASK_NULL;

	/* Initialize the data structure */
	cntrl->ntask = 0;
	cntrl->CPUS = num_cpus;
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
        #pragma omp parallel num_threads(num_cpus) firstprivate(num_cpus) 
        {
        #pragma omp single nowait
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

	 		#pragma omp taskgroup
                	{
                	   // Distribute the task between the CPUs
                	   int n_th=0;
                	   for(n_th=0; n_th< cntrl->CPUS; n_th++){
                	         #pragma omp task firstprivate(n_th) 
               	         {
               	            #ifdef DEBUG
               	               printf("\n Thread: %d computing a task. \n",omp_get_thread_num());
               	            #endif
               	            	   // Execute task
				   CAL_wrapper_CPU( run, num_cpus, n_th);
        	                }
        	           }
        	         }
                	#pragma omp taskwait
			// Couunting the number of executed kernels
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
		// Task for detaching a tile
	    }	 
  	  }
       }
     }
#ifdef DEBUG
  printf("Destruido\n");
#endif

  // Once the controller is detroyed, clean the queue and unset locks
  omp_set_lock(&(cntrl->lockQueue));
  CAL_TaskQueue_clean(&(cntrl->listTask));
  omp_unset_lock(&(cntrl->lockQueue));
  omp_unset_lock(&(cntrl->destroyCntrl));

   }

}


/*
 * Destroy the controller
 */

void CAL_CntrlCPUDestroy(CALCntrlCPU* cntrl){

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


// TODO, add this features to tasks
/*
 * Attach tiles in a controller
 */
void CAL_CntrlCPUAttach(CALCntrlCPU* cntrl, HitTile* tile){

}

/*
 * Dettach tiles in a controller
 */
void CAL_CntrlCPUDetach(CALCntrlCPU* cntrl, HitTile* tile){
        

}





