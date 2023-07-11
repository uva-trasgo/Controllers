//@cond INTERNAL
/**
 * @file Ctrl_PYNQ.c
 * @author Trasgo Group
 * @brief Source code for PYNQ backend.
 * @version 2.1
 * @date 2021-04-26
 * 
 * @copyright This software is provided to enhance knowledge and encourage progress in the scientific
 * community. It should be used only for research and educational purposes. Any reproduction
 * or use for commercial purpose, public redistribution, in source or binary forms, with or 
 * without modifications, is NOT ALLOWED without the previous authorization of the copyright 
 * holder. The origin of this software must not be misrepresented; you must not claim that you
 * wrote the original software. If you use this software for any purpose (e.g. publication),
 * a reference to the software package and the authors must be included.
 * 
 * @copyright THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @copyright Copyright (c) 2007-2020, Trasgo Group, Universidad de Valladolid.
 * All rights reserved.
 * 
 * @copyright More information on http://trasgo.infor.uva.es/
 */

#include "Architectures/Pynq/Ctrl_Pynq.h"

int enable = 1;
int disable = 0;
//int swap_depends_on_launch[N_RR];
int current_launch_task_id;
int current_host_task_id;
int evicted_host_task_id;
int host_task_ready = 1;
int currently_swapping = 0;

#ifdef _EXP_MODE
FILE * exp_file = NULL;
double init_time;
#endif


/*************************************************************
 ******** Prototypes of private functions ********************
 *************************************************************/

/**
 * Sync with main thread if queues are enabled, if queues are not enabled this function does nothing.
 * 
 * @param p_ctrl Pointer to the ctrl to perform the syncronization.
 */
void Ctrl_PYNQ_Sync(Ctrl_PYNQ *p_ctrl);

/**
 * Allocate memory for a new \e Ctrl_PYNQ_Tile.
 * 
 * @param p_ctrl Pointer to the ctrl to be attached to the tile.
 * @param p_task Pointer to the task containing the hitTile that will contain the new \e Ctrl_PYNQ_Tile.
 * 
 * @see Ctrl_PYNQ_EvalTaskAllocTile, Ctrl_PYNQ_Tile
 */
void Ctrl_PYNQ_CreateTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/**
 * Initializate a \e Ctrl_PYNQ_Tile.
 *
 * @param p_ctrl Pointer to the ctrl attached to the tile and to be updated of the initialization.
 * @param p_task Pointer to the task containing the HitTile that contains the \e Ctrl_PYNQ_Tile to be.
 * initialized.
 */
void Ctrl_PYNQ_InitTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/**
 * Perform memory transfer from host to device.
 * 
 * Pushes appropiate wait events and MoveTo task to the "move to" task queue.
 * 
 * @param p_ctrl Pointer to the ctrl attached to the tile to be moved.
 * @param p_tile Pointer to the task containing the tile to be moved.
 * 
 * @pre Memory transfers must be enabled on this ctrl.
 * @see Ctrl_PYNQ_EvalTaskMoveTo
 */
void Ctrl_PYNQ_EvalTaskMoveToInner(Ctrl_PYNQ *p_ctrl, HitTile *p_tile);

/**
 * Perform memory transfer from device to host.
 * 
 * Pushes appropiate wait events and MoveFrom task to the "move from" task queue.
 * 
 * @param p_ctrl Pointer to the ctrl attached to the tile to be moved.
 * @param p_tile Pointer to the task containing the tile to be moved.
 * 
 * @pre Memory transfers must be enabled on this ctrl.
 * @see Ctrl_PYNQ_EvalTaskMoveFrom
 */
void Ctrl_PYNQ_EvalTaskMoveFromInner(Ctrl_PYNQ *p_ctrl, HitTile *p_tile);

/**
 * Extract and evaluate tasks from \p p_stream until a task of type CTRL_TASK_TYPE_DESTROYCNTRL is found.
 * 
 * This function calls \e Ctrl_PYNQ_EvalTaskInner for the evaluation of tasks. This function is used for PYNQ kernel queue 
 * and PYNQ memory transfer queues.
 * 
 * @param p_stream Stream to extract the tasks from.
 * 
 * @see Ctrl_PYNQ_EvalTaskInner, Ctrl_PYNQ_ThreadInit
 */
void Ctrl_PYNQ_StreamConsume(Ctrl_TaskQueue *p_stream);

/**
 * Evaluation for inner PYNQ tasks.
 * 
 * Not to be confused with \e Ctrl_PYNQ_EvalTask, this function evaluates \e inner tasks, meaning that this is used from 
 * \e Ctrl_PYNQ_StreamConsume for tasks in PYNQ kernel and PYNQ trasnsfers queues.
 * 
 * @param p_task Task to be evaluated.
 * 
 * @see Ctrl_PYNQ_EvalTask, Ctrl_PYNQ_StreamConsume
 */
void Ctrl_PYNQ_EvalTaskInner(Ctrl_Task *p_task);

/*************************************************************
 ******** Prototypes of tasks' evaluation functions **********
 *************************************************************/

/**
 * Destroy a PYNQ ctrl.
 * @param p_ctrl Ctrl to be destroyed.
 */
void Ctrl_PYNQ_Destroy(Ctrl_PYNQ *p_ctrl);

/**
 * Evaluation of kernel launch.
 * 
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 * 
 * @see Ctrl_PYNQ_EvalTask, Ctrl_Launch
 */
void Ctrl_PYNQ_EvalTaskKernelLaunch(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of swap kernel launch.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 * 
 * @see Ctrl_PYNQ_EvalTask, Ctrl_Launch
 */
void Ctrl_PYNQ_EvalTaskSwapKernelLaunch(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of host task launch.
 * 
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 * 
 * @see Ctrl_PYNQ_EvalTask, Ctrl_HostTask
 */
void Ctrl_PYNQ_EvalTaskHostTaskLaunch(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of global sync. Waits for all work related to any tile attached to this ctrl. 
 * 
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 * 
 * @see Ctrl_PYNQ_EvalTask, Ctrl_GlobalSync
 */
void Ctrl_PYNQ_EvalTaskGlobalSync(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of creation of tiles.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_PYNQ_EvalTask, Ctrl_Domain
 */ 
void Ctrl_PYNQ_EvalTaskDomainTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of allocation of tiles.
 * 
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 * 
 * @see Ctrl_PYNQ_EvalTask, Ctrl_Alloc
 */
void Ctrl_PYNQ_EvalTaskAllocTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of subselecting tiles.
 *
 * @param p_ctrl: Ctrl in charge of task.
 * @param p_task: task to be evaluated.
 *
 * @see Ctrl_PYNQ_EvalTask, Ctrl_Select
 */
void Ctrl_PYNQ_EvalTaskSelectTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);
/**
 * Evaluation of freeing of tiles.
 * 
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 * 
 * @see Ctrl_PYNQ_EvalTask, Ctrl_Free
 */
void Ctrl_PYNQ_EvalTaskFreeTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of comunication of tile from host to device.
 * 
 * This function checks state of tile and if transfers are enabled, if comunication is necesary it calls to 
 * \e Ctrl_PYNQ_EvalTaskMoveToInner.
 * 
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 * 
 * @see Ctrl_PYNQ_EvalTask, Ctrl_PYNQ_EvalTaskMoveToInner, Ctrl_MoveTo
 */
void Ctrl_PYNQ_EvalTaskMoveTo(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of comunication of tile from device to host.
 * 
 * This function checks state of tile and if transfers are enabled, if comunication is necesary it calls to 
 * \e Ctrl_PYNQ_EvalTaskMoveToInner.
 * 
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 * 
 * @see Ctrl_PYNQ_EvalTask, Ctrl_PYNQ_EvalTaskMoveFromInner, Ctrl_MoveFrom
 */
void Ctrl_PYNQ_EvalTaskMoveFrom(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of wait. Waits for all the work related to the tile specified in \p p_task .
 * 
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 * 
 * @see Ctrl_PYNQ_EvalTask, Ctrl_WaitTile
 */
void Ctrl_PYNQ_EvalTaskWaitTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of change of dependance mode.
 * 
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated. .
 * 
 */
void Ctrl_PYNQ_EvalTaskSetDependanceMode(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/********************************************
 ******** PYNQ Controller functions **********
 ********************************************/

void Ctrl_PYNQ_Create(Ctrl_PYNQ *p_ctrl, Ctrl_Policy policy, int device) {
	p_ctrl->policy = policy;
	p_ctrl->p_tile_list_head = NULL;
	p_ctrl->p_tile_list_tail = NULL;
	p_ctrl->dependance_mode = CTRL_MODE_IMPLICIT;

	p_ctrl->event_seq=Ctrl_PYNQEvent_Create();

    setup_scheduler(0);


	//Alloc queues for kernel and memory transfers
    for(int _rr = 0; _rr < N_RR; _rr++) {
	    p_ctrl->p_kernel_stream[_rr]=(Ctrl_TaskQueue*)malloc(sizeof(Ctrl_TaskQueue));
	    Ctrl_TaskQueue_Init(p_ctrl->p_kernel_stream[_rr]);

        setup_reconfigurable_region(_rr);
    }
    generated_interrupts[N_RR] = 0;
#ifdef _DEBUG
    printf("After setup reconfigurable region\n");
#endif

    // TODO: remove this since we do not need transfer queues with the PYNQ
    p_ctrl->mem_moves = 1;
	if(p_ctrl->mem_moves){
		p_ctrl->p_moveTo_stream=(Ctrl_TaskQueue*)malloc(sizeof(Ctrl_TaskQueue));
		p_ctrl->p_moveFrom_stream=(Ctrl_TaskQueue*)malloc(sizeof(Ctrl_TaskQueue));
		Ctrl_TaskQueue_Init(p_ctrl->p_moveTo_stream);
		Ctrl_TaskQueue_Init(p_ctrl->p_moveFrom_stream);
    }
}

void Ctrl_PYNQ_EvalTask(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	switch ( p_task->task_type ) {
		case CTRL_TASK_TYPE_KERNEL:
			Ctrl_PYNQ_EvalTaskKernelLaunch(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_SWAP_KERNEL:
		#ifdef _DEBUG
		printf("----> EVAL SWAP TASK\n");
		#endif
			Ctrl_PYNQ_EvalTaskKernelLaunch(p_ctrl, p_task);
            //Ctrl_PYNQ_EvalTaskSwapKernelLaunch(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_KERNEL_SYNC:
			Ctrl_PYNQ_EvalTaskKernelLaunch(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_HOST:
			Ctrl_PYNQ_EvalTaskHostTaskLaunch(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_MIGRATE_HOST:
			Ctrl_PYNQ_EvalTaskHostTaskLaunch(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_GLOBALSYNC:
			Ctrl_PYNQ_EvalTaskGlobalSync(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_ALLOCTILE:
			Ctrl_PYNQ_EvalTaskAllocTile(p_ctrl,p_task);
			break;
		case CTRL_TASK_TYPE_DOMAINTILE:
			Ctrl_PYNQ_EvalTaskDomainTile(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_SELECTTILE:
			Ctrl_PYNQ_EvalTaskSelectTile(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_FREETILE:
			Ctrl_PYNQ_EvalTaskFreeTile(p_ctrl,p_task);
			break;
		case CTRL_TASK_TYPE_MOVETO:
			Ctrl_PYNQ_EvalTaskMoveTo(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_MOVEFROM:
			Ctrl_PYNQ_EvalTaskMoveFrom(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_WAITTILE:
			Ctrl_PYNQ_EvalTaskWaitTile(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_DESTROYCNTRL:
			Ctrl_PYNQ_Destroy(p_ctrl);
			break;
		case CTRL_TASK_TYPE_SETDEPENDANCEMODE:
			Ctrl_PYNQ_EvalTaskSetDependanceMode(p_ctrl, p_task);
			break;
		default:
			fprintf(stderr, "[Ctrl_PYNQ] Unsupported task type: %d.\n", p_task->task_type);
			exit(EXIT_FAILURE);
	}
}

int Ctrl_PYNQ_GetNumThreads(Ctrl_PYNQ *p_ctrl){
    return 2 + N_RR; // TODO: 2 for transfer queues. Maybe remove in the future as this is a zero-copy backend
}

void Ctrl_PYNQ_ThreadInit(Ctrl_PYNQ *p_ctrl, hwloc_topology_t topo, int node){
	//index of the thread used to execute kernels
	#ifdef _CTRL_QUEUE_
	int kernel_thread=1;
	#else
	int kernel_thread=0;
	#endif //_CTRL_QUEUE_

    cpu_set_t set;
	if(omp_get_thread_num() >= kernel_thread && omp_get_thread_num()<kernel_thread+N_RR){			//kernel thread	
		//bind thread
		//if (!hwloc_bitmap_iszero(p_ctrl->device_PYNQset)){
		//	hwloc_set_PYNQbind(topo, p_ctrl->device_PYNQset, HWLOC_PYNQBIND_THREAD);
		//}
		//hwloc_obj_t obj = hwloc_get_obj_below_by_type(topo,HWLOC_OBJ_NUMANODE, node, HWLOC_OBJ_CORE, 0);
        //if (obj){
        //    hwloc_set_cpubind(topo, obj->cpuset, HWLOC_CPUBIND_THREAD);
        //}
		//start executing tasks from the queue
        CPU_ZERO(&set);
        CPU_SET(1, &set);
#ifdef _DEBUG
        if (sched_setaffinity(0, sizeof(set), &set) == -1)
                   printf("sched_setaffinity");
#else
        sched_setaffinity(0, sizeof(set), &set);
#endif
		Ctrl_PYNQ_StreamConsume(p_ctrl->p_kernel_stream[omp_get_thread_num()]);

	}else if (omp_get_thread_num()==kernel_thread+N_RR){	//host to device
		//bind thread
		//hwloc_obj_t obj = hwloc_get_obj_below_by_type(topo,HWLOC_OBJ_NUMANODE, node, HWLOC_OBJ_CORE, 0); 
        //if (obj){
        //    hwloc_set_cpubind(topo, obj->cpuset, HWLOC_CPUBIND_THREAD);
        //}
		//start executing tasks from the queue
        CPU_ZERO(&set);
        CPU_SET(1, &set);
#ifdef _DEBUG
        if (sched_setaffinity(0, sizeof(set), &set) == -1)
                   printf("sched_setaffinity");
#else
        sched_setaffinity(0, sizeof(set), &set);
#endif
		Ctrl_PYNQ_StreamConsume(p_ctrl->p_moveTo_stream);

	}else if (omp_get_thread_num()==kernel_thread+1+N_RR){	//device to host
		//bind thread
		//hwloc_obj_t obj = hwloc_get_obj_below_by_type(topo,HWLOC_OBJ_NUMANODE, node, HWLOC_OBJ_CORE, 0);
        //if (obj){
        //    hwloc_set_cpubind(topo, obj->cpuset, HWLOC_CPUBIND_THREAD);
        //}
		//start executing tasks from the queue
        CPU_ZERO(&set);
        CPU_SET(1, &set);
#ifdef _DEBUG
        if (sched_setaffinity(0, sizeof(set), &set) == -1)
                   printf("sched_setaffinity");
#else
        sched_setaffinity(0, sizeof(set), &set);
#endif
		Ctrl_PYNQ_StreamConsume(p_ctrl->p_moveFrom_stream);
	}
}

/*********************************
 ******* Private functions *******
 *********************************/

void Ctrl_PYNQ_Sync(Ctrl_PYNQ *p_ctrl){
	#ifdef _CTRL_QUEUE_
	omp_set_lock(p_ctrl->p_lock_first_ctrl);
	omp_unset_lock(p_ctrl->p_lock_ctrl);
	omp_set_lock(p_ctrl->p_lock_host);
	omp_unset_lock(p_ctrl->p_lock_first_ctrl);

	omp_set_lock(p_ctrl->p_lock_first_host);
	omp_unset_lock(p_ctrl->p_lock_host);
	omp_set_lock(p_ctrl->p_lock_ctrl);
	omp_unset_lock(p_ctrl->p_lock_first_host);
	#endif //_CTRL_QUEUE_
}

void Ctrl_PYNQ_CreateTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	HitTile *p_tile = (HitTile *)(p_task->p_tile);
	Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)malloc(sizeof(Ctrl_PYNQ_Tile));

	p_tile_data->p_ctrl = p_ctrl;

	p_tile_data->host_status = CTRL_TILE_UNALLOC;
	p_tile_data->device_status = CTRL_TILE_UNALLOC;

	p_tile->ext = (void *)p_tile_data;

	p_tile_data->p_parent_ext = NULL;

	p_tile_data->is_initialized = false;
}

void Ctrl_PYNQ_InitTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	HitTile *p_tile = (HitTile *)(p_task->p_tile);
	Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_tile->ext);	

	//create node for the the new tile
	Ctrl_PYNQ_Tile_List *p_list_node = (Ctrl_PYNQ_Tile_List *)malloc(sizeof(Ctrl_PYNQ_Tile_List));
	p_list_node->p_prev = NULL;
	p_list_node->p_next = NULL;
	p_list_node->p_tile_ext = p_tile_data;
	p_tile_data->p_tile_elem = p_list_node;

	//insert node into the linked list of tiles
	if (p_ctrl->p_tile_list_tail != NULL){
		p_ctrl->p_tile_list_tail->p_next = p_list_node;
		p_list_node->p_prev = p_ctrl->p_tile_list_tail;
		p_ctrl->p_tile_list_tail = p_list_node;
	} else {
		p_ctrl->p_tile_list_head = p_ctrl->p_tile_list_tail = p_list_node;
	}

	p_tile_data->host_last_read_event= Ctrl_PYNQEvent_Create();
	p_tile_data->host_last_write_event= Ctrl_PYNQEvent_Create();

	if(p_ctrl->mem_moves){
		p_tile_data->kernel_last_read_event= Ctrl_PYNQEvent_Create();
		p_tile_data->kernel_last_write_event= Ctrl_PYNQEvent_Create();

		p_tile_data->offloading_last_read_event= Ctrl_PYNQEvent_Create();
		p_tile_data->offloading_last_write_event= Ctrl_PYNQEvent_Create();
	}

	p_tile_data->is_initialized = true;
}

void Ctrl_PYNQ_EvalTaskMoveToInner(Ctrl_PYNQ *p_ctrl, HitTile *p_tile) {
	Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_tile->ext);
	Ctrl_GenericEvent move_event = CTRL_GENERIC_EVENT_NULL;
	move_event.event_type = CTRL_EVENT_TYPE_PYNQ;

	//Wait for appropiate events
	move_event.event.event_pynq=p_tile_data->kernel_last_read_event;
	Ctrl_GenericEvent_StreamWait(move_event, p_ctrl->p_moveTo_stream);

	move_event.event.event_pynq=p_tile_data->kernel_last_write_event;
	Ctrl_GenericEvent_StreamWait(move_event, p_ctrl->p_moveTo_stream);

	move_event.event.event_pynq=p_tile_data->offloading_last_read_event;
	Ctrl_GenericEvent_StreamWait(move_event, p_ctrl->p_moveTo_stream);

	move_event.event.event_pynq=p_tile_data->host_last_write_event;
	Ctrl_GenericEvent_StreamWait(move_event, p_ctrl->p_moveTo_stream);

	//wait for previous task to finish if policy is sync
	if (p_ctrl->policy == CTRL_POLICY_SYNC) {
		move_event.event.event_pynq=p_ctrl->event_seq;
		Ctrl_GenericEvent_StreamWait(move_event, p_ctrl->p_moveTo_stream);
	}

	Ctrl_Task task=CTRL_TASK_NULL;
	task.task_type=CTRL_TASK_TYPE_MOVETO;
	task.p_tile=p_tile;

	//Push moveFrom task to moveFrom execution queue
	Ctrl_TaskQueue_Push(p_ctrl->p_moveTo_stream, task);

	//Update events
	Ctrl_PYNQEvent_Record(&p_tile_data->offloading_last_write_event, p_ctrl->p_moveTo_stream);
	
	if (p_ctrl->policy == CTRL_POLICY_SYNC) {
		Ctrl_PYNQEvent_Record(&p_ctrl->event_seq, p_ctrl->p_moveTo_stream);
	}

	//Update state of the tile
	p_tile_data->host_status = CTRL_TILE_VALID;
	p_tile_data->device_status = CTRL_TILE_VALID;
}

void Ctrl_PYNQ_EvalTaskMoveFromInner(Ctrl_PYNQ *p_ctrl, HitTile *p_tile) {
	Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_tile->ext);
	Ctrl_GenericEvent move_event = CTRL_GENERIC_EVENT_NULL;
	move_event.event_type = CTRL_EVENT_TYPE_PYNQ;

	//Wait for appropiate events
	move_event.event.event_pynq=p_tile_data->kernel_last_write_event;
	Ctrl_GenericEvent_StreamWait(move_event, p_ctrl->p_moveFrom_stream);
	
	move_event.event.event_pynq=p_tile_data->offloading_last_write_event;
	Ctrl_GenericEvent_StreamWait(move_event, p_ctrl->p_moveFrom_stream);
	
	move_event.event.event_pynq=p_tile_data->host_last_read_event;
	Ctrl_GenericEvent_StreamWait(move_event, p_ctrl->p_moveFrom_stream);
	
	move_event.event.event_pynq=p_tile_data->host_last_write_event;
	Ctrl_GenericEvent_StreamWait(move_event, p_ctrl->p_moveFrom_stream);

	//wait for previous task to finish if policy is sync
	if (p_ctrl->policy == CTRL_POLICY_SYNC) {
		move_event.event.event_pynq=p_ctrl->event_seq;
		Ctrl_GenericEvent_StreamWait(move_event, p_ctrl->p_moveFrom_stream);
	}

	Ctrl_Task task=CTRL_TASK_NULL;
	task.task_type=CTRL_TASK_TYPE_MOVEFROM;
	task.p_tile=p_tile;
	//Push moveFrom task to moveFrom execution queue
	Ctrl_TaskQueue_Push(p_ctrl->p_moveFrom_stream, task);

	//Update events
	Ctrl_PYNQEvent_Record(&p_tile_data->offloading_last_read_event, p_ctrl->p_moveFrom_stream);

	if (p_ctrl->policy == CTRL_POLICY_SYNC) {
		Ctrl_PYNQEvent_Record(&p_ctrl->event_seq, p_ctrl->p_moveFrom_stream);
	}

	//Update state of the tile
	p_tile_data->host_status = CTRL_TILE_VALID;
	p_tile_data->device_status = CTRL_TILE_VALID;
}

void Ctrl_PYNQ_StreamConsume(Ctrl_TaskQueue *p_stream){
    bool finish = false;
    while (!finish) {
		//Extract task from queue
        Ctrl_Task *p_task = NULL;
        p_task = Ctrl_TaskQueue_Pop(p_stream);
		
        if (p_task->task_type == CTRL_TASK_TYPE_DESTROYCNTRL) {
            finish = true;
        } else {
			//Evaluate task
			#ifdef _DEBUG
			printf("-----> EVALTASKINNER. type: %d, tid: %d\n", p_task->task_type, omp_get_thread_num());
			#endif
            Ctrl_PYNQ_EvalTaskInner(p_task);
			//Update task counter
            #pragma omp atomic update
                p_stream->last_finished++;
			//Free task (kernel tasks must be destroyed at the end)
			if (p_task->task_type != CTRL_TASK_TYPE_KERNEL && p_task->task_type != CTRL_TASK_TYPE_SWAP_KERNEL){
            	Ctrl_TaskQueue_FreeTask(p_task);
			}
        }
    }
    Ctrl_TaskQueue_Destroy(p_stream);
    free(p_stream); }

void Ctrl_PYNQ_EvalTaskInner(Ctrl_Task *p_task){
	switch ( p_task->task_type ) {
		case CTRL_TASK_TYPE_KERNEL:
            p_task->pfn_kernel_wrapper(p_task->request, p_task->device_id, p_task->ctrl_type, p_task->threads, p_task->blocksize, p_task->p_arguments);
			break;
        case CTRL_TASK_TYPE_SWAP_KERNEL:
#ifdef _DEBUG
            printf("From EvalTaskInner. bitstream: %s\n", p_task->bitstream_path);
#endif
            p_task->pfn_swapKernel_wrapper(p_task->swap_id, p_task->rr, p_task->bitstream_path);
            break;
		case CTRL_TASK_TYPE_KERNEL_SYNC:
			//p_task->pfn_kernelsync_wrapper();
			break;

		case CTRL_TASK_TYPE_MOVETO:
			{Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_task->p_tile->ext);
			HitTile *p_tile = p_task->p_tile;
			/* TODO: STRIDED TILES */


			//} else {
			//	fprintf(stderr, "[Ctrl_PYNQ] error: Number of dimensions not supported for non-owner tile in MoveTo: %d\n",
			//		p_tile->shape.info.sig.numDims);
			//}
			break;}
		case CTRL_TASK_TYPE_MOVEFROM:
			{Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_task->p_tile->ext);
			HitTile *p_tile = p_task->p_tile;
			/* TODO: STRIDED TILES */

			/* TILES WITH THEIR OWN MEMORY ALLOCATION, OR CONTIGUOUS 1D TILES NEED ONLY ONE CONTIGUOUS COPY */
			if ((p_tile->memStatus == HIT_MS_OWNER) || (p_tile->shape.info.sig.numDims == 1)) {
				memcpy(p_tile->data, p_tile_data->p_device_data, ((size_t)(p_tile->acumCard)) * (p_tile->baseExtent));
			}
			/* CONTIGUOUS 2D TILES */
			else if (p_tile->shape.info.sig.numDims == 2) {
				for (int i=0; i<p_tile->card[0]; i++) {
					memcpy(
						(void *)(p_tile->data + (p_tile->baseExtent) * (i*p_tile->origAcumCard[1])),
						(void *)(p_tile_data->p_device_data + (p_tile->baseExtent) * (i*p_tile->origAcumCard[1])),
						((size_t)(p_tile->card[1])) * (p_tile->baseExtent)
					);
				}
			}
			/* CONTIGUOUS 3D TILES */
			// TODO: Check if 3D transfers are correct! - Manu 04/2021
			else if (p_tile->shape.info.sig.numDims == 3) {
				for (int i=0; i<p_tile->card[0]; i++) {
					for (int j=0; j<p_tile->card[1]; j++) {
						memcpy(
							(void *)(p_tile->data + (p_tile->baseExtent) * (i*p_tile->origAcumCard[1] + j*p_tile->origAcumCard[2])),
							(void *)(p_tile_data->p_device_data + (p_tile->baseExtent) * (i*p_tile->origAcumCard[1] + j*p_tile->origAcumCard[2])),
							((size_t)(p_tile->card[2])) * (p_tile->baseExtent)
						);
					}
				}
			} else {
				fprintf(stderr, "[Ctrl_PYNQ] error: Number of dimensions not supported for non-owner tile in MoveFrom: %d\n",
					p_tile->shape.info.sig.numDims);
			}
			break;}
        case CTRL_TASK_TYPE_WAITEVENT:
            Ctrl_GenericEvent_Wait(p_task->event);
            break;
        case CTRL_TASK_TYPE_SIGNALEVENT:
            Ctrl_GenericEvent_Signal(p_task->event);
            break;
        case CTRL_TASK_TYPE_RELEASEEVENT:
            Ctrl_GenericEvent_Release(p_task->event);
            break;
        default:
            fprintf(stderr, "[Ctrl_PYNQ] EvalTaskInner: task type %d not implemented on this stream\n", p_task->task_type);
            exit(EXIT_FAILURE);
	}
}

/**********************************
 ** TASKS'S EVALUATION FUNCTIONS **
 **********************************/

void Ctrl_PYNQ_Destroy(Ctrl_PYNQ *p_ctrl) {
	// send destroy task to kernel and host tasks streams
	Ctrl_Task task=CTRL_TASK_NULL;
	task.task_type=CTRL_TASK_TYPE_DESTROYCNTRL;
    for(int _rr = 0; _rr < N_RR; _rr++) {
	    Ctrl_TaskQueue_Push(p_ctrl->p_kernel_stream[_rr],task);
    }
	Ctrl_TaskQueue_Push(p_ctrl_host_stream,task);
	
	// send destroy task to transfer streams 
	if (p_ctrl->mem_moves) {
		Ctrl_TaskQueue_Push(p_ctrl->p_moveFrom_stream,task);
		Ctrl_TaskQueue_Push(p_ctrl->p_moveTo_stream,task);
	}

	//Clean Pointers to lined list of tasks (should be empty by now)
	p_ctrl->p_tile_list_head = NULL;
	p_ctrl->p_tile_list_tail = NULL;

	//Free reamaining fields from Ctrl
	Ctrl_PYNQEvent_Destroy(&p_ctrl->event_seq);
	//hwloc_bitmap_free(p_ctrl->device_PYNQset);
}

void Ctrl_PYNQ_EvalTaskGlobalSync(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	// wait for all events from all tiles
	for (Ctrl_PYNQ_Tile_List *p_aux = p_ctrl->p_tile_list_head; p_aux != NULL; p_aux = p_aux->p_next) {
		Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_aux->p_tile_ext);

		Ctrl_PYNQEvent_Wait(p_tile_data->host_last_read_event);
		Ctrl_PYNQEvent_Wait(p_tile_data->host_last_write_event);
		if(p_ctrl->mem_moves){
			Ctrl_PYNQEvent_Wait(p_tile_data->kernel_last_read_event);
			Ctrl_PYNQEvent_Wait(p_tile_data->kernel_last_write_event);

			Ctrl_PYNQEvent_Wait(p_tile_data->offloading_last_read_event);
			Ctrl_PYNQEvent_Wait(p_tile_data->offloading_last_write_event);
		}
	}
	Ctrl_PYNQ_Sync(p_ctrl);
}

void Ctrl_PYNQ_EvalTaskSwapKernelLaunch(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	Ctrl_GenericEvent kernel_event = CTRL_GENERIC_EVENT_NULL;
	kernel_event.event_type = CTRL_EVENT_TYPE_PYNQ;

    p_task->ctrl_type=CTRL_TYPE_PYNQ;

	//wait for previous task to finish if policy is sync
	if(p_ctrl->policy==CTRL_POLICY_SYNC){
		kernel_event.event.event_pynq=p_ctrl->event_seq;
		Ctrl_GenericEvent_StreamWait(kernel_event, p_ctrl->p_kernel_stream[p_task->rr]);
	}

#ifdef _DEBUG
    printf("###### PUSHING SWAP TASK \n");
#endif
	//push kernel task to kernel exec queue
	Ctrl_TaskQueue_Push(p_ctrl->p_kernel_stream[p_task->rr],*p_task);
}

void Ctrl_PYNQ_EvalTaskKernelLaunch(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	Ctrl_GenericEvent kernel_event = CTRL_GENERIC_EVENT_NULL;
	kernel_event.event_type = CTRL_EVENT_TYPE_PYNQ;

	//wait for appropiate events from arguments according to their role
	for (int i = 0; i < p_task->n_arguments; i++) {
		if (p_task->p_roles[i] != KERNEL_INVAL) {
			HitTile *p_tile = (HitTile *)(p_task->pp_pointers[i]);
			KHitTile *p_ktile = (KHitTile *)((uint8_t *)p_task->p_arguments + p_task->p_displacements[i]);
			Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_tile->ext);

			if (hit_tileIsNull(*p_tile)) {
				fprintf(stderr, "Warning: Launching task, skipping null tile on parameter %d\n", i); fflush(stderr);
				continue;
			}

			if (p_tile_data->device_status == CTRL_TILE_UNALLOC && p_tile_data->host_status == CTRL_TILE_UNALLOC) {
				fprintf(stderr, "[Ctrl_PYNQ] Internal Error: Tile with no memory allocated as argument to kernel %d\n", i); fflush(stderr);
				exit(EXIT_FAILURE);
			}

			if(p_ctrl->mem_moves){
				if (p_tile_data->device_status == CTRL_TILE_UNALLOC) {
					fprintf(stderr, "[Ctrl_PYNQ] Warning: Launching kernel with tile with no device memory as argument, using host image instead %d\n", i); fflush(stderr);
				}

				if (p_task->p_roles[i] != KERNEL_OUT && p_tile_data->device_status == CTRL_TILE_INVALID) {
					if (p_tile_data->host_status != CTRL_TILE_VALID) {
						fprintf(stderr, "[Ctrl_PYNQ] Warning: Tile with uninitialized data as input on kernel %d\n", i); fflush(stderr);
					} else {
						//if tile's role is IN or IO, is not updated on device and host has memory allocated transfer it
						if (p_ctrl->dependance_mode == CTRL_MODE_IMPLICIT) {
							Ctrl_PYNQ_EvalTaskMoveToInner(p_ctrl, p_tile);
						}
					}
				}

			    p_ktile->data = p_tile->data;
                p_ktile->pynq_shared_memory = p_tile_data->pynq_shared_memory;

				kernel_event.event.event_pynq = p_tile_data->kernel_last_write_event;
				Ctrl_GenericEvent_StreamWait(kernel_event, p_ctrl->p_kernel_stream[p_task->rr]);

				kernel_event.event.event_pynq = p_tile_data->offloading_last_write_event;
				Ctrl_GenericEvent_StreamWait(kernel_event, p_ctrl->p_kernel_stream[p_task->rr]);

				if (p_task->p_roles[i] != KERNEL_IN) {
					kernel_event.event.event_pynq = p_tile_data->kernel_last_read_event;
					Ctrl_GenericEvent_StreamWait(kernel_event, p_ctrl->p_kernel_stream[p_task->rr]);

					kernel_event.event.event_pynq = p_tile_data->offloading_last_read_event;
					Ctrl_GenericEvent_StreamWait(kernel_event, p_ctrl->p_kernel_stream[p_task->rr]);
				}
			}else{
				if (p_task->p_roles[i] != KERNEL_OUT && p_tile_data->device_status == CTRL_TILE_INVALID) {
					fprintf(stderr, "[Ctrl_PYNQ] Warning: Tile with uninitialized data as input on kernel %d\n", i); fflush(stderr);
				}

				kernel_event.event.event_pynq = p_tile_data->host_last_write_event;
				Ctrl_GenericEvent_StreamWait(kernel_event, p_ctrl->p_kernel_stream[p_task->rr]);

				if (p_task->p_roles[i] != KERNEL_IN) {
					kernel_event.event.event_pynq = p_tile_data->host_last_read_event;
					Ctrl_GenericEvent_StreamWait(kernel_event, p_ctrl->p_kernel_stream[p_task->rr]);
				}
			}
		}
	}
	//create request with info for kernel execution
	Ctrl_Request request;
	request.pynq.n_cores = 1;
	request.pynq.n_arguments = p_task->n_arguments;
    request.pynq.p_roles = p_task->p_roles;
    request.pynq.p_displacements = p_task->p_displacements;
    request.pynq.rr = p_task->rr;
    request.pynq.launch_id = p_task->launch_id;

	p_task->request=request;
	p_task->ctrl_type=CTRL_TYPE_PYNQ;

	//wait for previous task to finish if policy is sync
	if(p_ctrl->policy==CTRL_POLICY_SYNC){
		kernel_event.event.event_pynq=p_ctrl->event_seq;
		Ctrl_GenericEvent_StreamWait(kernel_event, p_ctrl->p_kernel_stream[p_task->rr]);
	}

	//push kernel task to kernel exec queue
	#ifdef _DEBUG
	printf("Before pushing task of type: %d\n", p_task->task_type);
	#endif // _DEBUG
	Ctrl_TaskQueue_Push(p_ctrl->p_kernel_stream[p_task->rr],*p_task);

	//update events on arguments according to their roles
	for (int i = 0; i < p_task->n_arguments; i++) {
		if (p_task->p_roles[i] != KERNEL_INVAL) {
			HitTile *p_tile = (HitTile *)(p_task->pp_pointers[i]);
			Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_tile->ext);

			if(p_ctrl->mem_moves){
				if (p_task->p_roles[i] != KERNEL_IN) {
					p_tile_data->device_status = CTRL_TILE_VALID;
					if(p_tile_data->host_status == CTRL_TILE_VALID) p_tile_data->host_status = CTRL_TILE_INVALID;
					Ctrl_PYNQEvent_Record(&p_tile_data->kernel_last_write_event, p_ctrl->p_kernel_stream[p_task->rr]);
				}

				if (p_task->p_roles[i] != KERNEL_OUT) {
					Ctrl_PYNQEvent_Record(&p_tile_data->kernel_last_read_event, p_ctrl->p_kernel_stream[p_task->rr]);
				}
			}else{
				if (p_task->p_roles[i] != KERNEL_IN) {
					p_tile_data->device_status = CTRL_TILE_VALID;
					p_tile_data->host_status = CTRL_TILE_VALID;
					Ctrl_PYNQEvent_Record(&p_tile_data->host_last_write_event, p_ctrl->p_kernel_stream[p_task->rr]);
				}

				if (p_task->p_roles[i] != KERNEL_OUT) {
					Ctrl_PYNQEvent_Record(&p_tile_data->host_last_read_event, p_ctrl->p_kernel_stream[p_task->rr]);
				}
			}
		}
	}

	if(p_ctrl->policy==CTRL_POLICY_SYNC){
		Ctrl_PYNQEvent_Record(&p_ctrl->event_seq, p_ctrl->p_kernel_stream[p_task->rr]);
	}
}

void Ctrl_PYNQ_EvalTaskHostTaskLaunch(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	Ctrl_GenericEvent host_task_event = CTRL_GENERIC_EVENT_NULL;
	host_task_event.event_type = CTRL_EVENT_TYPE_PYNQ;

	//wait for appropiate events from arguments according to their role
	for (int i = 0; i < p_task->n_arguments; i++) {
		if (p_task->p_roles[i] != KERNEL_INVAL) {
			HitTile *p_tile = (HitTile *)(p_task->pp_pointers[i]);
			Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_tile->ext);

			if (p_tile_data->device_status == CTRL_TILE_UNALLOC && p_tile_data->host_status == CTRL_TILE_UNALLOC) {
				fprintf(stderr, "[Ctrl_PYNQ] Internal Error: Tile with no memory allocated as argument to host task %d\n", i); fflush(stderr);
				exit(EXIT_FAILURE);
			}

			if(p_ctrl->mem_moves) {
				if (p_tile_data->host_status == CTRL_TILE_UNALLOC) {
					fprintf(stderr, "[Ctrl_PYNQ] Warning: Launching host task with tile with no host memory as argument, using device image instead %d\n", i); fflush(stderr);
				}
				
				if (p_task->p_roles[i] != KERNEL_OUT && p_tile_data->host_status == CTRL_TILE_INVALID) {
					if (p_tile_data->device_status != CTRL_TILE_VALID) {
						fprintf(stderr, "[Ctrl_PYNQ] Warning: Tile with uninitialized data as input on host task %d\n", i); fflush(stderr);
					} else {
						//if tile's role is IN or IO, is not updated on host and device is valid transfer it
						if (p_ctrl->dependance_mode == CTRL_MODE_IMPLICIT) {
							Ctrl_PYNQ_EvalTaskMoveFromInner(p_ctrl, p_tile);
						}
					}
				}

				host_task_event.event.event_pynq = p_tile_data->offloading_last_read_event;
				Ctrl_GenericEvent_StreamWait(host_task_event, p_ctrl_host_stream);
			} else {
				if (p_task->p_roles[i] != KERNEL_OUT && p_tile_data->host_status == CTRL_TILE_INVALID) {
					fprintf(stderr, "[Ctrl_PYNQ] Warning: Tile with uninitialized data as input on kernel %d\n", i); fflush(stderr);
				}
			}

			host_task_event.event.event_pynq = p_tile_data->host_last_write_event;
			Ctrl_GenericEvent_StreamWait(host_task_event, p_ctrl_host_stream);

			if (p_task->p_roles[i] != KERNEL_IN) {
				p_tile_data->host_status = CTRL_TILE_VALID;
				if(p_ctrl->mem_moves){
					if(p_tile_data->device_status == CTRL_TILE_VALID) p_tile_data->device_status = CTRL_TILE_INVALID;
					host_task_event.event.event_pynq = p_tile_data->offloading_last_write_event;
					Ctrl_GenericEvent_StreamWait(host_task_event, p_ctrl_host_stream);
				} else {
					p_tile_data->device_status = CTRL_TILE_VALID;
				}
				
				host_task_event.event.event_pynq = p_tile_data->host_last_read_event;
				Ctrl_GenericEvent_StreamWait(host_task_event, p_ctrl_host_stream);
			}
		}
	}

	//wait for previous task to finish if policy is sync
	if(p_ctrl->policy==CTRL_POLICY_SYNC){
		host_task_event.event.event_pynq=p_ctrl->event_seq;
		Ctrl_GenericEvent_StreamWait(host_task_event, p_ctrl_host_stream);
	}

	//push host task to host exec queue
	Ctrl_TaskQueue_Push(p_ctrl_host_stream,*p_task);

	//update events on arguments according to their roles
	for (int i = 0; i < p_task->n_arguments; i++) {
		if (p_task->p_roles[i] != KERNEL_INVAL) {
			HitTile *p_tile = (HitTile *)(p_task->pp_pointers[i]);
			Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_tile->ext);
			if (p_task->p_roles[i] != KERNEL_IN) {
				Ctrl_PYNQEvent_Record(&p_tile_data->host_last_write_event, p_ctrl_host_stream);
			}

			if (p_task->p_roles[i] != KERNEL_OUT) {
				Ctrl_PYNQEvent_Record(&p_tile_data->host_last_read_event, p_ctrl_host_stream);
			}
		}
	}

	//wait for host task to finish if policy is sync
	if (p_ctrl->policy == CTRL_POLICY_SYNC) {
		Ctrl_PYNQEvent_Record(&p_ctrl->event_seq, p_ctrl_host_stream);
		Ctrl_PYNQEvent_Wait(p_ctrl->event_seq);
		Ctrl_PYNQ_Sync(p_ctrl);
	}
}

void Ctrl_PYNQ_EvalTaskDomainTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	Ctrl_PYNQ_CreateTile(p_ctrl, p_task);
	Ctrl_PYNQ_Sync(p_ctrl);
}

void Ctrl_PYNQ_EvalTaskAllocTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	HitTile *p_tile = (HitTile *)(p_task->p_tile);
	Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_tile->ext);

	if ( ! (p_tile_data->is_initialized) )
		Ctrl_PYNQ_InitTile(p_ctrl, p_task);

	p_tile_data->mem_flags = p_task->flags;

	if (p_tile_data->mem_flags & (CTRL_MEM_ALLOC_BOTH | CTRL_MEM_ALLOC_HOST) || !(p_tile_data->mem_flags & CTRL_MEM_ALLOC_DEV)) {
		//Allocate memory for the host image of the data inside the tile, equivalent to hit_tileAlloc(p_tile);
        PYNQ_allocatedSharedMemory(&((Ctrl_PYNQ_Tile *)p_tile->ext)->pynq_shared_memory, p_tile->acumCard * p_tile->baseExtent, 1);
        //p_tile->data = (&((Ctrl_PYNQ_Tile *)p_tile->ext)->pynq_shared_memory)->pointer;
        p_tile->data = ((Ctrl_PYNQ_Tile *)p_tile->ext)->pynq_shared_memory.pointer;

		//p_tile->data=(void*)malloc((size_t)p_tile->acumCard * p_tile->baseExtent);
		p_tile->memPtr = p_tile->data;
		p_tile_data->host_status = CTRL_TILE_INVALID;
		// Set device pointer same as host, overwritten if alloc on dev and mem moves later
		p_tile_data->p_device_data=p_tile->data;
		if (!p_ctrl->mem_moves || !(p_tile_data->mem_flags & CTRL_MEM_ALLOC_HOST)) {
			p_tile_data->device_status = CTRL_TILE_INVALID;
		}
		
	}
	
	if ((p_tile_data->mem_flags & CTRL_MEM_ALLOC_DEV) || (p_ctrl->mem_moves && ((p_tile_data->mem_flags & CTRL_MEM_ALLOC_BOTH) || !(p_tile_data->mem_flags & CTRL_MEM_ALLOC_HOST)))) {
		//Alloc memory for device image of the data
		//p_tile_data->p_device_data=(void*)hwloc_alloc_membind(p_ctrl->topo, (size_t)p_tile->acumCard * p_tile->baseExtent, p_ctrl->device_PYNQset, HWLOC_MEMBIND_BIND, 0);
		p_tile_data->device_status = CTRL_TILE_INVALID;
		if ((p_tile_data->mem_flags & CTRL_MEM_ALLOC_DEV) && !p_ctrl->mem_moves) {
			p_tile->data = p_tile_data->p_device_data;
			p_tile_data->host_status = CTRL_TILE_INVALID;
		}
	}

	Ctrl_PYNQ_Sync(p_ctrl);
}

void Ctrl_PYNQ_EvalTaskSelectTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	Ctrl_PYNQ_CreateTile(p_ctrl, p_task);

	HitTile *p_tile = (HitTile *)(p_task->p_tile);

	if ( hit_tileIsNull( *p_tile ) ) {
		Ctrl_PYNQ_Sync(p_ctrl);
		return;
	}

	HitTile *p_parent = p_tile->ref;

	Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_tile->ext);

	p_tile_data->p_parent_ext = ((Ctrl_PYNQ_Tile *)(p_parent->ext));

	Ctrl_PYNQ_Tile *p_parent_data = p_tile_data->p_parent_ext;

	if ((p_task->flags & CTRL_SELECT_INIT) == CTRL_SELECT_INIT) {
		Ctrl_PYNQ_InitTile(p_ctrl, p_task);
	}

	if (p_tile->memStatus == HIT_MS_NOT_OWNER) {
		p_tile_data->host_status   = p_tile_data->p_parent_ext->host_status;
		p_tile_data->device_status = p_tile_data->p_parent_ext->device_status;

		p_tile_data->p_device_data = p_parent_data->p_device_data + (p_tile->data - p_parent->data);
	}

	Ctrl_PYNQ_Sync(p_ctrl);
}

void Ctrl_PYNQ_EvalTaskFreeTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	HitTile *p_tile = (HitTile *)(p_task->p_tile);
	Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_tile->ext);

	if (p_tile_data->is_initialized) {
		//wait for the work related to the tile to finish
		Ctrl_PYNQEvent_Wait(p_tile_data->host_last_write_event);
		Ctrl_PYNQEvent_Wait(p_tile_data->host_last_read_event);
		if(p_ctrl->mem_moves){
			Ctrl_PYNQEvent_Wait(p_tile_data->kernel_last_read_event);
			Ctrl_PYNQEvent_Wait(p_tile_data->kernel_last_write_event);

			Ctrl_PYNQEvent_Wait(p_tile_data->offloading_last_read_event);
			Ctrl_PYNQEvent_Wait(p_tile_data->offloading_last_write_event);
		}

		// Destroy events of this tile
		Ctrl_PYNQEvent_Destroy(&p_tile_data->host_last_write_event);
		Ctrl_PYNQEvent_Destroy(&p_tile_data->host_last_read_event);
		if(p_ctrl->mem_moves){
			Ctrl_PYNQEvent_Destroy(&p_tile_data->kernel_last_read_event);
			Ctrl_PYNQEvent_Destroy(&p_tile_data->kernel_last_write_event);

			Ctrl_PYNQEvent_Destroy(&p_tile_data->offloading_last_read_event);
			Ctrl_PYNQEvent_Destroy(&p_tile_data->offloading_last_write_event);
		}

		//Remove tle from tile linked list 
		if (p_tile_data->p_tile_elem->p_prev != NULL) {
			if (p_tile_data->p_tile_elem->p_next == NULL) {
				p_tile_data->p_tile_elem->p_prev->p_next = NULL;
				p_ctrl->p_tile_list_tail = p_tile_data->p_tile_elem->p_prev;
			} else {
				p_tile_data->p_tile_elem->p_prev->p_next = p_tile_data->p_tile_elem->p_next;
			}
		}

		if (p_tile_data->p_tile_elem->p_next != NULL) {
			if (p_tile_data->p_tile_elem->p_prev == NULL) {
				p_tile_data->p_tile_elem->p_next->p_prev = NULL;
				p_ctrl->p_tile_list_head = p_tile_data->p_tile_elem->p_next;
			} else {
				p_tile_data->p_tile_elem->p_next->p_prev = p_tile_data->p_tile_elem->p_prev;
			}
		}

		//Clear node fields
		p_tile_data->p_tile_elem->p_tile_ext = NULL;
		p_tile_data->p_tile_elem->p_next = NULL;
		p_tile_data->p_tile_elem->p_prev = NULL;

		//Free node
		free(p_tile_data->p_tile_elem);
	}

	if (p_tile->memStatus == HIT_MS_OWNER) {
		//Free host image of the tile, equivalent to hit_tileFree(*p_tile);
        	PYNQ_freeSharedMemory(&p_tile_data->pynq_shared_memory);

		p_tile->memPtr = NULL;
		p_tile->data = NULL;
		p_tile->memStatus = HIT_MS_NOMEM;
	}

	//Clear tile fields
	p_tile->ext = NULL;
	p_tile_data->p_ctrl = NULL;

	// Free tile
	free(p_tile_data);

	Ctrl_PYNQ_Sync(p_ctrl);
}

void Ctrl_PYNQ_EvalTaskMoveTo(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	HitTile *p_tile = (HitTile *)(p_task->p_tile);
	Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_tile->ext);

	if (hit_tileIsNull(*p_tile)) return;

	//If transfers are active and tile is not updated perform the transfer
	if(p_ctrl->mem_moves){
		if (p_tile_data->host_status == CTRL_TILE_UNALLOC) {
			fprintf(stderr, "[Ctrl_PYNQ] Internal Error: Tryinng to move tile from host to device but host memory was not allocated\n"); fflush(stderr);
			exit(EXIT_FAILURE);
		}

		if (p_tile_data->device_status == CTRL_TILE_UNALLOC) {
			fprintf(stderr, "[Ctrl_PYNQ] Internal Error: Tryinng to move tile from host to device but device memory was not allocated\n"); fflush(stderr);
			exit(EXIT_FAILURE);
		}

		if (p_tile_data->host_status == CTRL_TILE_INVALID) {
			fprintf(stderr, "[Ctrl_PYNQ] Warning: Moving a tile from host to device with invalid data on host memory\n"); fflush(stderr);
		}

		//If tile is not updated perform the transfer
		if (!(p_tile_data->host_status == CTRL_TILE_VALID && p_tile_data->device_status == CTRL_TILE_VALID) || p_ctrl->dependance_mode == CTRL_MODE_EXPLICIT) {
			Ctrl_PYNQ_EvalTaskMoveToInner(p_ctrl, p_tile);
		}
	}

	//If policy is sync wait for transfer to finish
	if (p_ctrl->policy == CTRL_POLICY_SYNC) {
		Ctrl_PYNQEvent_Wait(p_ctrl->event_seq);
		Ctrl_PYNQ_Sync(p_ctrl);
	}
}

void Ctrl_PYNQ_EvalTaskMoveFrom(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	HitTile *p_tile = (HitTile *)(p_task->p_tile);
	Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_tile->ext);

	if (hit_tileIsNull(*p_tile)) return;

	//If transfers are active and tile is not updated perform the transfer
	if(p_ctrl->mem_moves){
		if (p_tile_data->host_status == CTRL_TILE_UNALLOC) {
			fprintf(stderr, "[Ctrl_PYNQ] Internal Error: Trying to move tile from device to host but host memory was not allocated\n"); fflush(stderr);
			exit(EXIT_FAILURE);
		}

		if (p_tile_data->device_status == CTRL_TILE_UNALLOC) {
			fprintf(stderr, "[Ctrl_PYNQ] Internal Error: Trying to move tile from device to host but device memory was not allocated\n"); fflush(stderr);
			exit(EXIT_FAILURE);
		}

		if (p_tile_data->device_status == CTRL_TILE_INVALID) {
			fprintf(stderr, "[Ctrl_PYNQ] Warning: Moving a tile from device to host with invalid data on device memory\n"); fflush(stderr);
		}
		
		if (!(p_tile_data->host_status == CTRL_TILE_VALID && p_tile_data->device_status == CTRL_TILE_VALID) || p_ctrl->dependance_mode == CTRL_MODE_EXPLICIT) {
			Ctrl_PYNQ_EvalTaskMoveFromInner(p_ctrl, p_tile);
		}
	}

	//If policy is sync wait for transfer to finish
	if (p_ctrl->policy == CTRL_POLICY_SYNC) {
		Ctrl_PYNQEvent_Wait(p_ctrl->event_seq);
		Ctrl_PYNQ_Sync(p_ctrl);
	}
}

void Ctrl_PYNQ_EvalTaskWaitTile(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	HitTile *p_tile = (HitTile *)(p_task->p_tile);
	Ctrl_PYNQ_Tile *p_tile_data = (Ctrl_PYNQ_Tile *)(p_tile->ext);

	if (hit_tileIsNull(*p_tile)) return;

	//Wait for all work related to this tile to finish 
	Ctrl_PYNQEvent_Wait(p_tile_data->host_last_write_event);
	Ctrl_PYNQEvent_Wait(p_tile_data->host_last_read_event);
	if(p_ctrl->mem_moves){
		Ctrl_PYNQEvent_Wait(p_tile_data->kernel_last_read_event);
		Ctrl_PYNQEvent_Wait(p_tile_data->kernel_last_write_event);

		Ctrl_PYNQEvent_Wait(p_tile_data->offloading_last_read_event);
		Ctrl_PYNQEvent_Wait(p_tile_data->offloading_last_write_event);
	}
	Ctrl_PYNQ_Sync(p_ctrl);
}

void Ctrl_PYNQ_EvalTaskSetDependanceMode(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task) {
	p_ctrl->dependance_mode = p_task->flags;
	if (p_ctrl->policy == CTRL_POLICY_SYNC){
		Ctrl_PYNQ_Sync(p_ctrl);
	}
}

int last_swap_id = 0;
int swap_id_to_run = 0;
//int currently_swapping[N_RR];

void Ctrl_KernelWrapper_KernelSync() {

}

void Ctrl_KernelWrapper_Swap(int swap_id, int rr, char * bitstream_path) {
	omp_set_lock(&lock_swap);
    int read_swap_id_to_run;
#ifdef _DEBUG
    printf("(Wrapper_Swap) Before getting turn to swap\n");
#endif
    //do { 
    //    #pragma omp atomic read
    //    read_swap_id_to_run = swap_id_to_run;
    //} while(read_swap_id_to_run != swap_id);

#ifdef _DEBUG
    printf("Before getting lock_swap\n");
#endif
   
#ifdef _DEBUG
    printf("(Wrapper_Swap) Before getting lock_RR %d\n", rr);
#endif

#ifdef _DEBUG
    printf("(Wrapper Swap) Before getting lock wait\n");
#endif
#ifdef _DEBUG
    printf("Hello from KernelWrapper_Swap: %s, rr: %d\n", bitstream_path, rr);
#endif

   
    // Reset RR before swapping 
    PYNQ_writeGPIO(&RR[rr].reset, &enable);
    PYNQ_writeGPIO(&RR[rr].reset, &disable);

    PYNQ_writeGPIO(&RR[rr].decouple_rst_clk_gpio, &enable);


#ifdef _DEBUG
    for(int i = 0; i < 10; i++)
        printf("(1) Waiting after reset\n");
#endif

    int decouple_status;
    PYNQ_writeGPIO(&RR[rr].decouple, &enable);
    PYNQ_readGPIO(&RR[rr].decouple_status, &decouple_status);
#ifdef _DEBUG
    printf("(Wrapper_Swap) before, decouple_status: %d\n", decouple_status);
#endif
#ifdef _EXP_MODE
    double begin_swap_time = omp_get_wtime();
#endif
    PYNQ_loadBitstream(bitstream_path, 1);
#ifdef _EXP_MODE
	double current_time = omp_get_wtime();
    fprintf(exp_file, "{ \"op\":\"partial_swap\", \"bitstream_path\":\"%s\", \"start_time\":%lf, \"finish_time\":%lf, \"rr\": %d }\n",
	 bitstream_path, begin_swap_time - init_time, current_time - init_time, rr);
#endif

    PYNQ_writeGPIO(&RR[rr].decouple_rst_clk_gpio, &disable);

    // Reset RR again after swapping
    PYNQ_writeGPIO(&RR[rr].reset, &enable);
    PYNQ_writeGPIO(&RR[rr].reset, &disable);

 

    PYNQ_closeHLS(&RR[rr].state);
    PYNQ_openHLS(&RR[rr].state, rr_address[rr], MM_AXI_SIZE);
    //setup_reconfigurable_region(rr);
    // Decouple once reset has been applied
    PYNQ_writeGPIO(&RR[rr].decouple, &disable);
    PYNQ_readGPIO(&RR[rr].decouple_status, &decouple_status);
#ifdef _DEBUG
    printf("(Wrapper_Swap) after, decouple_status: %d\n", decouple_status);
#endif
#ifdef _DEBUG
    for(int i = 0; i < 10; i++)
        printf("(2) Waiting after reset\n");
    printf("After reset rr: %d\n", rr);
#endif

    clean_bram(rr);

#ifdef _DEBUG
    printf("After cleaning bram rr: %d\n", rr);
#endif


    setup_hls_interrupts(rr);
#ifdef _DEBUG
    printf("Bye from KernelWrapper_Swap\n");
#endif



	//#pragma omp atomic write seq_cst
	//currently_swapping = 0;
	//#pragma omp flush(currently_swapping)

	// #pragma omp atomic write seq_cst
	// RR[rr].busy = 0;
	// #pragma omp flush(RR)


    free(bitstream_path);

	omp_unset_lock(&lock_swap);

    //context_switch_thread();
}

//#include "Kernel/Ctrl_KernelArgs.h"
//Ctrl_Task Ctrl_KernelTaskCreate_Swap( int rr, char * bitstream_path, CTRL_KERNEL_SKIP_IMPL( TYPED_POINTER, NULL, 1, PYNQ, DEFAULT, 1, OUT, HitTile_, dummy_tile ) ) {
//#ifdef _DEBUG
//    printf("(Create_Swap) RR %d, %s\n", rr, bitstream_path);
//#endif
//    Ctrl_Task task = CTRL_TASK_NULL;
//    task.task_type = CTRL_TASK_TYPE_SWAP_KERNEL;
//    task.pfn_swapKernel_wrapper = Ctrl_KernelWrapper_Swap;
//    task.n_arguments = 0;
//    task.rr = rr;
//
//	///* Tile with role OUT used only for synchronisation and emptying the queue. This way swap
//	// * operations can be run before the call to wait_finish() in the next iteration happens. */
//	//CTRL_KERNEL_SKIP_IMPL( ARG_LIST_CREATE_KTILE, task.p_arguments, 1, PYNQ, DEFAULT, 1, OUT, HitTile_int, dummy_tile); 
//	//CTRL_KERNEL_SKIP_IMPL( ROLES, task.p_roles, n_implementations, IN, HitTile_int, dummy_tile); 
//	//CTRL_KERNEL_SKIP_IMPL( POINTERS, task.pp_pointers, n_implementations, IN, HitTile_int, dummy_tile); 
//	//CTRL_KERNEL_SKIP_IMPL( DISPLACEMENTS, task.p_displacements, n_implementations, IN, HitTile_int, dummy_tile); 
//
//    //task.bitstream_path = bitstream_path;
//    task.bitstream_path = (char *)malloc(512 * sizeof(char));
//    memcpy(task.bitstream_path, bitstream_path, 512 * sizeof(char));
//    task.swap_id = last_swap_id++;
//
//    //#pragma omp atomic write
//    //currently_swapping[rr] = 1;
//
//    // The lock should be here so that wait_finish() is not run before the swap operation happens.
//    // Otherwise a deadlock will happen as the swap operation is necessary for the following kernel
//    // launch to occur.
//    return task; 
//} 






/////////////////////////////////////////////
// SCHEDULER FUNCTIONS
////////////////////////////////////////////


void setup_scheduler(int restart_tasks) {
    //setup_dma();
    bram_ctrl_address[0] = 0x40020000;
    bram_ctrl_address[1] = 0x40022000;

    rr_address[0] = 0x40000000;
    rr_address[1] = 0x40010000;

    // Arguments offsets
    ktile_wrapper_args_offset[0] = 0x10;
    ktile_wrapper_args_offset[1] = 0x40;
    ktile_wrapper_args_offset[2] = 0x70;

    ktile_data_args_offset[0] = 0x34;
    ktile_data_args_offset[1] = 0x64;
    ktile_data_args_offset[2] = 0x94;

    int_args_offset[0] = 0xA0;
    int_args_offset[1] = 0xA8;
    int_args_offset[2] = 0xB0;
    int_args_offset[3] = 0xB8;
    int_args_offset[4] = 0xC0;
    int_args_offset[5] = 0xC8;
    int_args_offset[6] = 0xD0;
    int_args_offset[7] = 0xD8;

    float_args_offset[0] = 0xE0;
    float_args_offset[1] = 0xE8;
    float_args_offset[2] = 0xF0;
    float_args_offset[3] = 0xF8;
    float_args_offset[4] = 0x100;
    float_args_offset[5] = 0x108;
    float_args_offset[6] = 0x110;
    float_args_offset[7] = 0x118;

    return_offset = 0x120;

    for(int rr = 0; rr < N_RR; rr++) {
        PYNQ_createMMIOWindow(&BRAMController[rr].bram, bram_ctrl_address[rr], BRAM_SIZE);
        clean_bram(rr);
    }

    for(int rr = 0; rr < N_RR+1; rr++) {
        for(int i = 0; i < N_SAVED_VARS; i++) {
            RR[rr].context.var[i] = 0;
            RR[rr].context.init_var[i] = 0;
            RR[rr].context.incr_var[i] = 0;
            RR[rr].context.saved[i] = 0;
        }
        RR[rr].context.valid = 0;
    }

    PYNQ_openInterruptController(&InterruptManager.interrupt_controller, INTERRUPT_CTRL_ADDRESS);
    PYNQ_openUIO(&InterruptManager.uio_state, 0); 


    // We register the interrupts of the N_RR reconfigurable regions plus the host-task interrupt
    for(int rr = 0; rr < N_RR+1; rr++)
        PYNQ_registerInterrupt(&InterruptManager.interrupt_controller, rr, 1); 

    PYNQ_openGPIO(&RR[0].decouple, 960, GPIO_OUT);
    PYNQ_openGPIO(&RR[1].decouple, 961, GPIO_OUT);
    PYNQ_openGPIO(&RR[0].reset, 962, GPIO_OUT);
    PYNQ_openGPIO(&RR[1].reset, 963, GPIO_OUT);
    PYNQ_openGPIO(&RR[0].decouple_status, 964, GPIO_IN);
    PYNQ_openGPIO(&RR[1].decouple_status, 965, GPIO_IN);
    PYNQ_openGPIO(&host_interrupt, 966, GPIO_OUT);
    PYNQ_openGPIO(&RR[0].decouple_rst_clk_gpio, 967, GPIO_OUT);
    PYNQ_openGPIO(&RR[1].decouple_rst_clk_gpio, 968, GPIO_OUT);

    omp_init_lock(&lock_swap);
    omp_init_lock(&lock_wait);
    for(int rr = 0; rr < N_RR; rr++)
        omp_init_lock(&lock_RR[rr]);
}

#define H_NCOL (1024+2)

void setup_reconfigurable_region(int rr) {
    PYNQ_openHLS(&RR[rr].state, rr_address[rr], MM_AXI_SIZE);

    RR[rr].return_var = 0;
    generated_interrupts[rr] = 0;

    setup_hls_interrupts(rr);

	//#pragma omp atomic write
    //currently_swapping[rr] = 0;

	int read_currently_swapping;
	#pragma omp atomic write seq_cst
	currently_swapping = 0;
    //swap_depends_on_launch[rr] = 0;

	#pragma atomic write seq_cst
	RR[rr].busy = 0;
}

void setup_hls_interrupts(int rr) {
#ifdef _DEBUG
    printf("Hi from setup interrupts\n");
#endif
    PYNQ_writeToHLS(&RR[rr].state, &enable, GIER, sizeof(int));
#ifdef _DEBUG
    printf("After setting GIER\n");
#endif
    PYNQ_writeToHLS(&RR[rr].state, &enable, IP_IER, sizeof(int));
#ifdef _DEBUG
    printf("After setting IER\n");
#endif
}

void clean() {
    for(int rr = 0; rr < N_RR; rr++) {
        for(int i = 0; i < N_P_ARGS; i++) {
            PYNQ_freeSharedMemory(&RR[rr].ktile_data[i]);
        }
    }
}

void reset_interrupt_controller() {
    PYNQ_AXI_INTERRUPT_CONTROLLER * interrupt_controller = &InterruptManager.interrupt_controller;

    unsigned int raised_irqs, work, current_irq=0;
    int IER_val;

    // Read the interrupts
    PYNQ_readMMIO(&(interrupt_controller->mmio_window), &raised_irqs, 0x04, sizeof(unsigned int));

    // ACKNOWLEDGE INTERRUPTS
    // Acknowledge in HLS block
    int ISR_HLS = 1;

    // Which RR triggered the interrupt?
    work = raised_irqs;
    for(int _rr = 0; _rr < N_RR+1; _rr++) {
        if(work & 0x1) {
            // Reset only RRs. This does not apply to host-tasks
            if(_rr < N_RR)
                PYNQ_writeToHLS(&RR[_rr].state, &ISR_HLS, 0xC, sizeof(int));
            else
                PYNQ_writeGPIO(&host_interrupt, &disable);
            generated_interrupts[_rr] = 1;
        }
        work = work >> 1;
    }

    // Acknowledge in controller
    PYNQ_writeMMIO(&(interrupt_controller->mmio_window), &raised_irqs, 0x0C, sizeof(unsigned int));
}

#include <sys/time.h>

int wait_finish(struct timeval * timeout) {
	int read_currently_swapping;
    int swapping;

    // If a swap operation depends on an unfinished launch opearation allow wait to 
    // happen as otherwise this would cause deadlock

#ifdef _DEBUG
    printf("(wait_finish) Before checking swap_depends_on_launch\n");
#endif


#ifdef _DEBUG
    printf("(wait_finish) Before getting lock_wait\n");
#endif

    int n_interrupts;
	int err;
#ifdef _DEBUG
    printf("Before waitForUIO ");
#endif
    if(timeout->tv_sec == 0 && timeout->tv_usec == 0) {
#ifdef _DEBUG
        printf("no timeout\n");
#endif
        err = PYNQ_waitForUIO(&InterruptManager.uio_state, &n_interrupts);
    }
    else {
#ifdef _DEBUG
        printf("timeout\n");
#endif
        err = PYNQ_waitForUIO_timeout(&InterruptManager.uio_state, &n_interrupts, timeout);
    }

#ifdef _DEBUG
    printf("After waitForUIO\n");
#endif

    reset_interrupt_controller();

    return n_interrupts;
}

int get_return(int rr) {
    return RR[rr].return_var;
}

void clean_bram(int rr) {
    //int clear[MM_AXI_SIZE];
    //for(int i = 0; i < MM_AXI_SIZE; i++)
    //    clear[i] = 0;
   
    struct context clean_context; 

    for(int i = 0; i < N_SAVED_VARS; i++) {
        clean_context.var[i] = 0;
        clean_context.init_var[i] = 0;
        clean_context.incr_var[i] = 0;
        clean_context.saved[i] = 0;
    }
    clean_context.valid = 0;

    //PYNQ_writeMMIO(&BRAMController[rr].bram, clear, 0x0, BRAM_SIZE);
    if(rr < N_RR)
        PYNQ_writeMMIO(&BRAMController[rr].bram, &clean_context, 0x0, sizeof(struct context));
    else
        RR[rr].context = clean_context;
}

void reset(int rr) {
    PYNQ_writeGPIO(&RR[rr].reset, &enable);
    PYNQ_writeGPIO(&RR[rr].reset, &disable);

    PYNQ_readMMIO(&BRAMController[rr].bram, &RR[rr].context, 0x0, sizeof(struct context));
    PYNQ_readFromHLS(&RR[rr].state, &RR[rr].return_var, return_offset, sizeof(int));

    // One of the launched kernels will not finish yet
    n_launched_kernels--;

    setup_hls_interrupts(rr);
}

// For testing purposes only
void _print_output(int rr) {
    if(rr < N_RR) {
        int return_val;
        printf("(RR %d) return_val: %d\n", rr, get_return(rr));

        for(int i = 0; i < 10; i++) {
            PYNQ_readMMIO(&BRAMController[rr].bram, &return_val, 4*i, sizeof(int));
            printf("(RR %d) (bram) val: %d\n", rr, return_val);
        }   
    }
    else {
        for(int i = 0; i < N_SAVED_VARS; i++) {
            printf("(RR %d) (bram) val: %d\n", rr, RR[rr].context.var[i]);
            //RR[rr].context.var[i] = 0;
            //RR[rr].context.init_var[i] = 0;
            //RR[rr].context.incr_var[i] = 0;
            //RR[rr].context.saved[i] = 0;
        }
        //RR[rr].context.valid = 0;
    }
}
