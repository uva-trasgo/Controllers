///@cond INTERNAL
/**
 * @file Ctrl_Core.c
 * @author Trasgo Group
 * @brief Source code for core functions of Controllers.
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

#include "Core/Ctrl_Config.h"
#include "Core/Ctrl_Core.h"
#include <string.h>

/**
 * optional weights for distributed computations
 */
HitWeights ctrl_weights = {0, NULL};

/**
 * Contains the list of ctrls.
 */
Ctrl *p_ctrl_global = NULL;

/**
 * Size of global list of ctrls \e p_ctrl_global.
 */
int n_ctrls;

/**
 * Topology of the machine.
 */
hwloc_topology_t topo;

/**
 * Index of the numa node to use for host threads.
 */
int host_node = 0;

/**
 * Policy to be used by ctrls.
 */
Ctrl_Policy policy = CTRL_POLICY_DEFAULT;

Ctrl_TaskQueue *p_ctrl_host_stream = NULL;

/**
 * Evaluate \p p_task.
 *
 * This calls to the appropiate eval_task depending on \p p_ctrl's type.
 *
 * @param p_ctrl pointer to the ctrl the task was sent to.
 * @param p_task pointer to the task to be evaluated.
 *
 * @see Ctrl_Cpu_EvalTask, Ctrl_Cuda_EvalTask, Ctrl_Hip_EvalTask, Ctrl_OpenCLGpu_EvalTask
 */
void Ctrl_ExecTask(Ctrl *p_ctrl, Ctrl_Task *p_task);

/**
 * Create and execute a task of type \p type associated to \p p_ctrl .
 *
 * @param p_ctrl ctrl to send the task to.
 * @param type type of the task to send.
 * @param p_tile tile to place inside the task. This can be null for certain task's type.
 *
 * @see Ctrl_ExecTask, Ctrl_TaskQueue_Push
 */
void Ctrl_AddTask(Ctrl *p_ctrl, Ctrl_TaskType type, HitTile *p_tile);

/**
 * If ctrl queues are active add a task of type \p type that needs flags to \p p_ctrl inner queue, if ctrl queues are not
 * active pass that task to \e Ctrl_ExecTask instead.
 *
 * @param p_ctrl ctrl to send the task to.
 * @param type type of the task to send.
 * @param p_tile tile to place inside the task. This can be null for certain task's type.
 * @param flags flags to pass to the task.
 *
 * @see Ctrl_ExecTask, Ctrl_TaskQueue_Push, Ctrl_AllocInner, Ctrl_SelectInner
 */
void Ctrl_AddTaskFlagged(Ctrl *p_ctrl, Ctrl_TaskType type, HitTile *p_tile, int flags);

/**
 * Evaluation for host tasks and events by the host task thread.
 *
 * @param p_task task to evaluate.
 *
 * @see Ctrl_Thread_HostTask
 */
void Ctrl_EvalTask(Ctrl_Task *p_task);

/**
 * Function for extra threads for ctrls. If extra threads are needed for a ctrl, this function takes the spawner thread created
 * and divides it and distributes it as needed depending on the configuration of its ctrl.
 *
 * @see Ctrl_Thread_Init
 */
void Ctrl_Thread_Spawner();

/**
 * Function for host task thread. Creates host task queue and starts executing tasks from it until a destroy task arrives to the queue.
 *
 * @see Ctrl_Thread_Init, Ctrl_EvalTask
 */
void Ctrl_Thread_HostTask();

/**
 * Calculates the number of threads needed for \p p_ctrl.
 *
 * @param p_ctrl Ctrl we want to get the number of extra threads of.
 *
 * @return Number of threads needed for \p p_ctrl.
 */
int Ctrl_GetNumThreads(Ctrl *p_ctrl);

/**
 * Creates a ctrl with the args especified.
 *
 * @param type type of the ctrl to create
 * @param id id for the ctrl. Must be positive.
 * @param args string with the argument to the ctrl. Corresponds to a line of the config file for the ctrl_block
 *
 * @see __ctrl_block__
 */
void Ctrl_Create(Ctrl_Type type, int id, char *args);

int Ctrl_Thread_Init() {
	hwloc_obj_t obj;
	if (omp_get_thread_num() == 0) {
		// bind thread to first core
		if ((obj = hwloc_get_obj_below_by_type(topo, HWLOC_OBJ_NUMANODE, host_node, HWLOC_OBJ_CORE, 0))) {
			if (hwloc_set_cpubind(topo, obj->cpuset, HWLOC_CPUBIND_THREAD) != 0) {
				fprintf(stderr, "[Ctrl_Thread_Init] Warning enforcing affinity of main thread returned an error. Make sure you have permission to use those resources.\n");
			}
		} else {
			fprintf(stderr, "[Ctrl_Thread_Init] Warning core 0 of host NUMA node %d not found.\n", host_node);
		}
		fflush(stderr);

		return 0;
	} else if (omp_get_thread_num() == 1) { // host task executor
		// bind thread to core 1
		if ((obj = hwloc_get_obj_below_by_type(topo, HWLOC_OBJ_NUMANODE, host_node, HWLOC_OBJ_CORE, 1))) {
			if (hwloc_set_cpubind(topo, obj->cpuset, HWLOC_CPUBIND_THREAD) != 0)
				fprintf(stderr, "[Ctrl_Thread_Init] Warning enforcing affinity of host task thread returned an error. Make sure you have permission to use those resources.\n");
		} else {
			fprintf(stderr, "[Ctrl_Thread_Init] Warning core 1 of host NUMA node %d not found.\n", host_node);
		}
		fflush(stderr);

		Ctrl_Thread_HostTask();
	} else { // spawner threads
		Ctrl_Thread_Spawner();
	}

	return 1;
}

void Ctrl_Thread_HostTask() {
	// Extract and evaluate tasks until destroy task
	bool finish = false;
	while (!finish) {
		Ctrl_Task *p_task = Ctrl_TaskQueue_Pop(p_ctrl_host_stream);
		if (p_task->task_type == CTRL_TASK_TYPE_DESTROYCTRL) {
			finish = true;
		} else {
			Ctrl_EvalTask(p_task);
			// Update queue last_finished index
			#pragma omp atomic update
			p_ctrl_host_stream->last_finished++;
			// free stuff inside task
			if (p_task->task_type != CTRL_TASK_TYPE_HOST) {
				Ctrl_TaskQueue_FreeTask(p_task);
			}
		}
	}
	// Destroy queue (clean up)
	Ctrl_TaskQueue_Destroy(p_ctrl_host_stream);
	free(p_ctrl_host_stream);
}

void Ctrl_Thread_Spawner() {
	// TODO @sergioalo the -3 should really be a constant
	int   cpu_ctrl_counter = 0;
	PCtrl p_ctrl           = NULL;
	for (int i = 0; i < n_ctrls; i++) {
		p_ctrl = &p_ctrl_global[i];
		if (p_ctrl->type != CTRL_TYPE_CPU)
			continue;
		if (cpu_ctrl_counter == omp_get_thread_num() - 3)
			break;
		cpu_ctrl_counter++;
	}

	// Divide thread if ctrl needs more threads
	#pragma omp parallel num_threads(Ctrl_GetNumThreads(p_ctrl))
	{
		// Distribute new threads
		switch (p_ctrl->type) {
			#ifdef _CTRL_ARCH_CPU_
			case CTRL_TYPE_CPU:
				Ctrl_Cpu_ThreadInit(&(p_ctrl->p_impl->cpu), topo, host_node);
				break;
			#endif // _CTRL_ARCH_CPU_
			default:
				#pragma omp single
				{
					fprintf(stderr, "[Ctrl Warning] Ctrls of type %d don't know how to use the extra threads in __ctrl_block__!\n", p_ctrl->type);
					fflush(stderr);
				}
				break;
		}
	}
}

void Ctrl_Create(Ctrl_Type type, int id, char *args) {
	PCtrl p_ctrl   = &p_ctrl_global[id];
	p_ctrl->p_impl = (Ctrl_Impl *)malloc(sizeof(Ctrl_Impl));
	p_ctrl->id     = id;
	p_ctrl->type   = type;
	switch (type) {
		#ifdef _CTRL_ARCH_CPU_
		case CTRL_TYPE_CPU:
			p_ctrl->p_impl->cpu.topo = topo;
			Ctrl_Cpu_Create(&(p_ctrl->p_impl->cpu), policy, args);
			break;
		#endif // _CTRL_ARCH_CPU_

		#ifdef _CTRL_ARCH_CUDA_
		case CTRL_TYPE_CUDA:
			Ctrl_Cuda_Create(&(p_ctrl->p_impl->cuda), policy, args);
			break;
		#endif // _CTRL_ARCH_CUDA_

		#ifdef _CTRL_ARCH_HIP_
		case CTRL_TYPE_HIP:
			Ctrl_Hip_Create(&(p_ctrl->p_impl->hip), policy, args);
			break;
		#endif // _CTRL_ARCH_HIP_

		#ifdef _CTRL_ARCH_OPENCL_GPU_
		case CTRL_TYPE_OPENCL_GPU:
			Ctrl_OpenCLGpu_Create(&(p_ctrl->p_impl->opencl_gpu), policy, args);
			break;
		#endif // _CTRL_ARCH_OPENCL_GPU_

		#ifdef _CTRL_ARCH_FPGA_
		case CTRL_TYPE_FPGA:
			Ctrl_FPGA_Create(&(p_ctrl->p_impl->fpga), policy, args);
			break;
		#endif // _CTRL_ARCH_FPGA_
		default:
			fprintf(stderr, "[Ctrl_Create] Unknown or unsupported architecture: %d\n", type);
			exit(EXIT_FAILURE);
	}
}

PCtrl Ctrl_Get(int id) {
	if (id < 0 || id >= n_ctrls) {
		fprintf(stderr, "[Ctrl_Get] Invalid ctrl id %d\n", id);
		exit(EXIT_FAILURE);
	}
	return &p_ctrl_global[id];
}

int Ctrl_GetNCtrls() {
	return n_ctrls;
}

void Ctrl_ExecTask(Ctrl *p_ctrl, Ctrl_Task *p_task) {
	switch (p_ctrl->type) {
		#ifdef _CTRL_ARCH_CPU_
		case CTRL_TYPE_CPU:
			Ctrl_Cpu_EvalTask(&(p_ctrl->p_impl->cpu), p_task);
			break;
		#endif // _CTRL_ARCH_CPU_

		#ifdef _CTRL_ARCH_CUDA_
		case CTRL_TYPE_CUDA:
			Ctrl_Cuda_EvalTask(&(p_ctrl->p_impl->cuda), p_task);
			break;
		#endif // _CTRL_ARCH_CUDA_

		#ifdef _CTRL_ARCH_HIP_
		case CTRL_TYPE_HIP:
			Ctrl_Hip_EvalTask(&(p_ctrl->p_impl->hip), p_task);
			break;
		#endif // _CTRL_ARCH_HIP_

		#ifdef _CTRL_ARCH_OPENCL_GPU_
		case CTRL_TYPE_OPENCL_GPU:
			Ctrl_OpenCLGpu_EvalTask(&(p_ctrl->p_impl->opencl_gpu), p_task);
			break;
		#endif // _CTRL_ARCH_OPENCL_GPU_

		#ifdef _CTRL_ARCH_FPGA_
		case CTRL_TYPE_FPGA:
			Ctrl_FPGA_EvalTask(&(p_ctrl->p_impl->fpga), p_task);
			break;
		#endif // _CTRL_ARCH_FPGA_
		default:
			fprintf(stderr, "[Ctrl_ExecTask] Unsupported architecture. Recompile Ctrl library with the proper support.\n");
			exit(EXIT_FAILURE);
	}

	if (p_ctrl->type == CTRL_TYPE_CPU) {
		if (p_task->task_type != CTRL_TASK_TYPE_HOST && p_task->task_type != CTRL_TASK_TYPE_KERNEL) {
			Ctrl_TaskQueue_FreeTask(p_task);
		}
	} else {
		if (p_task->task_type != CTRL_TASK_TYPE_HOST) {
			Ctrl_TaskQueue_FreeTask(p_task);
		}
	}
}

void Ctrl_EndBlock() {
	for (int i = 0; i < n_ctrls; i++) {
		Ctrl_AddTask(&p_ctrl_global[i], CTRL_TASK_TYPE_DESTROYCTRL, NULL);
		free(p_ctrl_global[i].p_impl);
		p_ctrl_global[i].p_impl = NULL;
	}

	Ctrl_Task task = CTRL_TASK_NULL;
	task.task_type = CTRL_TASK_TYPE_DESTROYCTRL;
	Ctrl_TaskQueue_Push(p_ctrl_host_stream, task);
	free(p_ctrl_global);
	free(ctrl_weights.ratios);
	hwloc_topology_destroy(topo);
}

void Ctrl_AddTask(Ctrl *p_ctrl, Ctrl_TaskType type, HitTile *p_tile) {
	Ctrl_Task task = CTRL_TASK_NULL;
	task.task_type = type;

	task.p_tile = p_tile;

	Ctrl_ExecTask(p_ctrl, &task);
}

void Ctrl_AddTaskFlagged(Ctrl *p_ctrl, Ctrl_TaskType type, HitTile *p_tile, int flags) {
	Ctrl_Task task = CTRL_TASK_NULL;
	task.task_type = type;

	task.p_tile = p_tile;
	task.flags  = flags;

	Ctrl_ExecTask(p_ctrl, &task);
}

void Ctrl_LaunchKernel(Ctrl *p_ctrl, Ctrl_Task task) {
	Ctrl_ExecTask(p_ctrl, &task);
}

void Ctrl_LaunchHostTask(Ctrl *p_ctrl, Ctrl_Task task) {
	Ctrl_ExecTask(p_ctrl, &task);
}

void Ctrl_GlobalSync(Ctrl *p_ctrl) {
	Ctrl_AddTask(p_ctrl, CTRL_TASK_TYPE_GLOBALSYNC, NULL);
}

void Ctrl_AllocInner(Ctrl *p_ctrl, HitTile *p_tile, int flags) {
	/* UPDATES FOR SHADOW COPIES, OR ALLOCATION OF SELECTIONS OF NO-MEMORY VARIABLES */
	// ADJUST ORIG_ACUM_CARD TO CARDINALITIES TO TRANSFORM ON MEMORY_OWNER, AS IN HITMAP
	if ((p_tile->memStatus == HIT_MS_NOT_OWNER) || (p_tile->memStatus == HIT_MS_NOMEM)) {
		/* 3.1. NEW STRIDES TO ACCESS ARE ALWAYS 1 */
		for (int i = 0; i < hit_shapeDims(p_tile->shape); i++) {
			p_tile->qstride[i] = 1;
		}

		/* 3.2. UPDATE ORIGINAL ACUMULATED CARDINALITIES, NOW IT HAS ITS OWN MEMORY */
		// Code below extracted from the body of hit_tileUpdateAcumCards(p_tile);
		p_tile->origAcumCard[hit_shapeDims(p_tile->shape)] = 1;
		int cardinality                                    = 1;
		for (int i = (hit_shapeDims(p_tile->shape) - 1); i >= 0; i--) {
			cardinality             = cardinality * p_tile->card[i];
			p_tile->origAcumCard[i] = cardinality;
		}
		p_tile->acumCard = p_tile->origAcumCard[0];
	}
	p_tile->memStatus = HIT_MS_OWNER;

	// RE-ADJUST TO PITCHED CARDINALITIES
	#ifdef ALIGNED_SIZE
	// TODO: Check if this works. This code was written by Arturo.
	//  Manu 04/2021
	int dims = hit_tileDims(*p_tile);
	if (dims > 1) {
		size_t toPitch = p_tile->card[dims - 1] * p_tile->baseExtent;

		toPitch = ((toPitch + ALIGNED_SIZE - 1) / ALIGNED_SIZE) * ALIGNED_SIZE / p_tile->baseExtent;

		p_tile->origAcumCard[dims - 1] = toPitch;
		for (int i = dims - 2; i >= 0; i--) {
			toPtich *= p_tile->card[1];
			p_tile->origAcumCard[i] = toPitch;
		}
	}
	#endif

	Ctrl_AddTaskFlagged(p_ctrl, CTRL_TASK_TYPE_ALLOCTILE, p_tile, flags);
}

void Ctrl_SelectInner(Ctrl *p_ctrl, HitTile *p_tile, int flags) {
	Ctrl_AddTaskFlagged(p_ctrl, CTRL_TASK_TYPE_SELECTTILE, p_tile, flags);
}

void Ctrl_DomainInner(Ctrl *p_ctrl, HitTile *p_tile) {
	Ctrl_AddTask(p_ctrl, CTRL_TASK_TYPE_DOMAINTILE, p_tile);
}

void Ctrl_FreeInner(Ctrl *p_ctrl, HitTile *p_tile) {
	Ctrl_AddTask(p_ctrl, CTRL_TASK_TYPE_FREETILE, p_tile);
}

void Ctrl_MoveToInner(Ctrl *p_ctrl, HitTile *p_tile) {
	Ctrl_AddTask(p_ctrl, CTRL_TASK_TYPE_MOVETO, p_tile);
}

void Ctrl_MoveFromInner(Ctrl *p_ctrl, HitTile *p_tile) {
	Ctrl_AddTask(p_ctrl, CTRL_TASK_TYPE_MOVEFROM, p_tile);
}

void Ctrl_WaitTileInner(Ctrl *p_ctrl, HitTile *p_tile) {
	Ctrl_AddTask(p_ctrl, CTRL_TASK_TYPE_WAITTILE, p_tile);
}

void Ctrl_Hosttask_Sync() {
	Ctrl_TaskQueue_Synchronize(p_ctrl_host_stream);
}

void Ctrl_Synchronize() {
	for (int i = 0; i < n_ctrls; i++) {
		if (p_ctrl_global[i].p_impl != NULL) Ctrl_GlobalSync(&p_ctrl_global[i]);
	}

	Ctrl_Hosttask_Sync();
}

void Ctrl_SetDependanceMode(Ctrl *p_ctrl, int mode) {
	Ctrl_AddTaskFlagged(p_ctrl, CTRL_TASK_TYPE_SETDEPENDANCEMODE, NULL, mode);
}

Ctrl_Info Ctrl_GetInfo(Ctrl *p_ctrl) {
	Ctrl_Info info     = CTRL_INFO_NULL;
	info.host_affinity = host_node;
	switch (p_ctrl->type) {
		#ifdef _CTRL_ARCH_CPU_
		case CTRL_TYPE_CPU:
			Ctrl_Cpu_GetInfo(&(p_ctrl->p_impl->cpu), &info);
			break;
		#endif // _CTRL_ARCH_CPU_

		#ifdef _CTRL_ARCH_CUDA_
		case CTRL_TYPE_CUDA:
			Ctrl_Cuda_GetInfo(&(p_ctrl->p_impl->cuda), &info);
			break;
		#endif // _CTRL_ARCH_CUDA_

		#ifdef _CTRL_ARCH_HIP_
		case CTRL_TYPE_HIP:
			Ctrl_Hip_GetInfo(&(p_ctrl->p_impl->hip), &info);
			break;
		#endif // _CTRL_ARCH_HIP_

		#ifdef _CTRL_ARCH_OPENCL_GPU_
		case CTRL_TYPE_OPENCL_GPU:
			Ctrl_OpenCLGpu_GetInfo(&(p_ctrl->p_impl->opencl_gpu), &info);
			break;
		#endif // _CTRL_ARCH_OPENCL_GPU_

		#ifdef _CTRL_ARCH_FPGA_
		case CTRL_TYPE_FPGA:
			Ctrl_FPGA_GetInfo(&(p_ctrl->p_impl->fpga), &info);
			break;
		#endif // _CTRL_ARCH_FPGA_
		default:
			fprintf(stderr, "[Ctrl_GetInfo] Unsupported architecture %d. Recompile Ctrl library with the proper support.\n", p_ctrl->type);
			exit(EXIT_FAILURE);
	}
	return info;
}

double Ctrl_TimeLastOpInner(Ctrl *p_ctrl, HitTile *p_tile) {
	if (hit_tileIsNull(*p_tile)) return -1;

	// wait for last task in the tile to be done before getting it's duration
	Ctrl_WaitTileInner(p_ctrl, p_tile);
	switch (p_ctrl->type) {
		#ifdef _CTRL_ARCH_CPU_
		case CTRL_TYPE_CPU:
			return Ctrl_Cpu_TimeLastOp(&(p_ctrl->p_impl->cpu), p_tile);
		#endif // _CTRL_ARCH_CPU_

		#ifdef _CTRL_ARCH_CUDA_
		case CTRL_TYPE_CUDA:
			return Ctrl_Cuda_TimeLastOp(&(p_ctrl->p_impl->cuda), p_tile);
		#endif // _CTRL_ARCH_CUDA_

		#ifdef _CTRL_ARCH_HIP_
		case CTRL_TYPE_HIP:
			return Ctrl_Hip_TimeLastOp(&(p_ctrl->p_impl->hip), p_tile);
		#endif // _CTRL_ARCH_HIP_

		#ifdef _CTRL_ARCH_OPENCL_GPU_
		case CTRL_TYPE_OPENCL_GPU:
			return Ctrl_OpenCLGpu_TimeLastOp(&(p_ctrl->p_impl->opencl_gpu), p_tile);
		#endif // _CTRL_ARCH_OPENCL_GPU_

		#ifdef _CTRL_ARCH_FPGA_
		case CTRL_TYPE_FPGA:
			return Ctrl_FPGA_TimeLastOp(&(p_ctrl->p_impl->fpga), p_tile);
		#endif // _CTRL_ARCH_FPGA_
		default:
			fprintf(stderr, "[Ctrl_TimeLastOp] Unsupported architecture %d. Recompile Ctrl library with the proper support.\n", p_ctrl->type);
			exit(EXIT_FAILURE);
	}
}

int Ctrl_Dev(Ctrl_Type type, int *avail_impls, int n_impl) {
	int result = -1;
	switch (type) {
		case CTRL_TYPE_CPU:
			for (int i = 0; i < n_impl; i++) {
				if (avail_impls[i] > result) {
					if (CTRL_IMPL_IN_RANGE(avail_impls[i], GENERIC) || CTRL_IMPL_IN_RANGE(avail_impls[i], CPU)) {
						result = avail_impls[i];
					}
					#ifdef _CTRL_MKL_
					else if (avail_impls[i] == CPULIB_MKL) {
						result = avail_impls[i];
					}
					#endif // _CTRL_MKL_
				}
			}
			break;
		case CTRL_TYPE_CUDA:
			for (int i = 0; i < n_impl; i++) {
				if (avail_impls[i] > result) {
					if (CTRL_IMPL_IN_RANGE(avail_impls[i], GENERIC) || CTRL_IMPL_IN_RANGE(avail_impls[i], CUDA)) {
						result = avail_impls[i];
					}
					#ifdef _CTRL_CUBLAS_
					else if (avail_impls[i] == CUDALIB_CUBLAS) {
						result = avail_impls[i];
					}
					#endif // _CTRL_CUBLAS_
					#ifdef _CTRL_MAGMA_
					else if (avail_impls[i] == CUDALIB_MAGMA) {
						result = avail_impls[i];
					}
					#endif // _CTRL_MAGMA_
				}
			}
			break;
		case CTRL_TYPE_HIP:
			for (int i = 0; i < n_impl; i++) {
				if (avail_impls[i] > result) {
					if (CTRL_IMPL_IN_RANGE(avail_impls[i], GENERIC) || CTRL_IMPL_IN_RANGE(avail_impls[i], HIP)) {
						result = avail_impls[i];
					}
				}
			}
			break;
		case CTRL_TYPE_OPENCL_GPU:
			for (int i = 0; i < n_impl; i++) {
				if (avail_impls[i] > result) {
					if (CTRL_IMPL_IN_RANGE(avail_impls[i], GENERIC) || CTRL_IMPL_IN_RANGE(avail_impls[i], OPENCLGPU)) {
						result = avail_impls[i];
					}
				}
			}
			break;
		case CTRL_TYPE_FPGA:
			for (int i = 0; i < n_impl; i++) {
				if (avail_impls[i] > result) {
					if (CTRL_IMPL_IN_RANGE(avail_impls[i], FPGA)) {
						result = avail_impls[i];
					}
				}
			}
			break;
	}
	return result;
}

void Ctrl_EvalTask(Ctrl_Task *p_task) {
	// Evaluation of task
	switch (p_task->task_type) {
		case CTRL_TASK_TYPE_HOST:
			p_task->pfn_hostTask_wrapper(p_task->p_arguments);
			break;
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
			fprintf(stderr, "[Core EvalTask] Task type not implemented on this stream\n");
			exit(EXIT_FAILURE);
			break;
	}
}

/* returns the number of threads p_ctrl will need */
int Ctrl_GetNumThreads(Ctrl *p_ctrl) {
	int default_num_threads = 0;
	switch (p_ctrl->type) {
		#ifdef _CTRL_ARCH_CPU_
		case CTRL_TYPE_CPU:
			return default_num_threads + Ctrl_Cpu_GetNumThreads(&(p_ctrl->p_impl->cpu));
		#endif // _CTRL_ARCH_CPU_
		default:
			return default_num_threads;
	}
}

void Ctrl_SetPolicy(Ctrl_Policy p) {
	policy = p;
}

/**
 * Helper function to read a file to a string.
 *
 * @param file path to file
 * @return char* string with the contents of \p file . Must be freed after use.
 */
char *read_file(const char *file) {
	FILE *f = fopen(file, "r");

	if (!f) {
		fprintf(stderr, "ERROR: File for devices selection could not be opened: %s\n", file);
		exit(EXIT_FAILURE);
	}

	fseek(f, 0, SEEK_END);
	size_t length = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = (char *)malloc(length + 1);

	size_t read_length = fread(buffer, 1, length, f);
	if (length != read_length) {
		free(buffer);
		fprintf(stderr, "ERROR: Reading devices selection file: %s\n", file);
		exit(EXIT_FAILURE);
	}
	fclose(f);

	buffer[length] = '\0';
	return buffer;
}

void Ctrl_ParseConfig(const char *file) {
	// READ CONFIG FILE IN A STRING BUFFER
	char *buffer = read_file(file);

	// GET HOST NAME AND RANK IN NODE
	char *hostname     = hit_comNodeName();
	int   rank_in_node = hit_comNodeGroupRank();

	// WEIGHTS: GET NUMBER OF ACTIVE PROCESSES TO CREATE ARRAY
	HitTopology com_topo         = hit_topology(plug_topPlain);
	int         global_num_procs = hit_topDimCard(com_topo, 0);
	int         global_rank      = hit_topDimRank(com_topo, 0);
	float       com_weight       = 1.0f; // Default value
	ctrl_weights.ratios          = (float *)malloc(sizeof(float) * global_num_procs);
	for (int i = 0; i < global_num_procs; i++)
		ctrl_weights.ratios[global_rank] = 1.0f; // Default value
	ctrl_weights.num_procs = global_num_procs;
	hit_topFree(com_topo);

	// LOCATE START OF THIS MACHINE NAME SECTION
	char node_name[MPI_MAX_PROCESSOR_NAME + 6]; // extra space for the "NODE " string
	sprintf(node_name, "node %s", hostname);

	char *node_data = strstr(buffer, node_name);
	if (node_data == NULL) {
		// FALLBACK: TRY TO LOCATE WILDCARD NODE SECTION
		sprintf(node_name, "node *");
		node_data = strstr(buffer, node_name);

		if (node_data == NULL) {
			fflush(stdout);
			fprintf(stderr, "ERROR: Device selection file -- No NODE section in file %s for node %s\n", file, hostname);
			exit(EXIT_FAILURE);
		}
	}
	// LOCATE END OF THIS MACHINE NAME SECTION AND CLEAN THE REST OF THE BUFFER
	char *end_node_data = strstr(node_data + 4, "node ");
	if (end_node_data != NULL) *end_node_data = '\0';

	// LOCATE START OF THIS RANK SECTION
	char proc_str[10];
	sprintf(proc_str, "proc %d ", rank_in_node);
	char *rank_data = strstr(node_data, proc_str);

	char proc_str_noaff[10];
	sprintf(proc_str_noaff, "proc %d\n", rank_in_node);
	char *rank_data_noaff = strstr(node_data, proc_str_noaff);
	if (rank_data == NULL && rank_data_noaff == NULL) {
		fflush(stdout);
		fprintf(stderr, "ERROR: Device selection file -- No config found in file %s for rank %d of node %s\n", file, rank_in_node, hostname);
		exit(EXIT_FAILURE);
	} else if (rank_data != NULL && rank_data_noaff != NULL) {
		fflush(stdout);
		fprintf(stderr, "ERROR: Device selection file -- Multiple configs found in file %s for rank %d of node %s\n", file, rank_in_node, hostname);
		exit(EXIT_FAILURE);
	}
	rank_data = rank_data == NULL ? rank_data_noaff : rank_data;

	// LOCATE END OF THIS RANK SECTION AND CLEAN THE REST OF THE BUFFER
	char *end_rank_data = strstr(rank_data + 4, "proc ");
	if (end_rank_data != NULL) *end_rank_data = '\0';

	// AFFINITY: GET (OPTIONAL) NUMA NODE IDENTIFIER FOR HOST CODE
	// SKIP "PROC" STRING AND RANK ID
	int foo, aff;
	int count = sscanf(rank_data, "proc %d %d\n", &foo, &aff);
	if (count < 1) {
		fflush(stdout);
		fprintf(stderr, "INTERNAL ERROR: Device selection file -- Internal proc rank missed! %d\n", rank_in_node);
		exit(EXIT_FAILURE);
	}
	if (foo != rank_in_node) {
		fflush(stdout);
		fprintf(stderr, "INTERNAL ERROR: Proc rank has changed during parsing! %d != %d\n", rank_in_node, foo);
		exit(EXIT_FAILURE);
	}
	// AFFINITY SPECIFICATION FOUND FOR THIS RANK
	if (count == 2) host_node = aff;

	// CONSUME LINE
	rank_data = strstr(rank_data, "\n");
	if (rank_data == NULL) {
		fflush(stdout);
		fprintf(stderr, "ERROR: Device selection file -- No devices, premature end of section in file %s, node %s, rank %d\n", file, hostname, rank_in_node);
		exit(EXIT_FAILURE);
	}
	// SKIP NEW LINE CHAR
	rank_data++;

	// READ DEVICES
	Ctrl_Config *devs = (Ctrl_Config *)malloc(sizeof(Ctrl_Config));

	int ndevs   = 0;
	int ncpu    = 0;
	int ncuda   = 0;
	int nhip    = 0;
	int noclgpu = 0;
	int nfpga   = 0;

	for (char *tok = strtok(rank_data, " \t\n");
		 tok != NULL;
		 tok = strtok(NULL, " \t\n")) {

		ndevs++;
		devs = (Ctrl_Config *)realloc(devs, ndevs * sizeof(Ctrl_Config));

		if (!strcmp(tok, "cpu")) {
			ncpu++;
			devs[ndevs - 1] = (Ctrl_Config){.type = CTRL_TYPE_CPU, .args = strtok(NULL, "\n")};
		} else if (!strcmp(tok, "cuda")) {
			ncuda++;
			devs[ndevs - 1] = (Ctrl_Config){.type = CTRL_TYPE_CUDA, .args = strtok(NULL, "\n")};
		} else if (!strcmp(tok, "hip")) {
			nhip++;
			devs[ndevs - 1] = (Ctrl_Config){.type = CTRL_TYPE_HIP, .args = strtok(NULL, "\n")};
		} else if (!strcmp(tok, "opencl")) {
			noclgpu++;
			devs[ndevs - 1] = (Ctrl_Config){.type = CTRL_TYPE_OPENCL_GPU, .args = strtok(NULL, "\n")};
		} else if (!strcmp(tok, "fpga")) {
			nfpga++;
			devs[ndevs - 1] = (Ctrl_Config){.type = CTRL_TYPE_FPGA, .args = strtok(NULL, "\n")};
		} else if (!strcmp(tok, "weight")) {
			ndevs--; // This entry is not a device
			char *weight_str = strtok(NULL, "\n");
			int   ok         = sscanf(weight_str, "%f\n", &com_weight);
			if (ok != 1) {
				fflush(stdout);
				fprintf(stderr, "ERROR: Device selection file -- Non readable weight value in file %s, node %s, proc %d, value string: %s\n", file, hostname, rank_in_node, tok);
				exit(EXIT_FAILURE);
			}
		} else {
			fflush(stdout);
			fprintf(stderr, "ERROR: Device selection file -- Unknown device type in file %s, node %s, proc %d, device name: %s\n", file, hostname, rank_in_node, tok);
			exit(EXIT_FAILURE);
		}
	}

	// WEIGHTS: COMMUNICATE WEIGHTS
	MPI_Allgather(&com_weight, 1, MPI_FLOAT, ctrl_weights.ratios, 1, MPI_FLOAT, MPI_COMM_WORLD);

	// LOAD HARDWARE TOPOLOGY INFO
	hwloc_topology_init(&topo);
	hwloc_topology_set_flags(topo, HWLOC_TOPOLOGY_FLAG_INCLUDE_DISALLOWED);
	hwloc_topology_load(topo);

	// INITIALIZE CONTROLLERS LIST
	n_ctrls       = ndevs;
	p_ctrl_global = (Ctrl *)malloc(ndevs * sizeof(Ctrl));
	omp_set_num_threads(2 + ncpu);

	if (policy == CTRL_POLICY_DEFAULT) {
		if (getenv("CTRL_POLICY")) {
			if (!strcmp(getenv("CTRL_POLICY"), "SYNC")) {
				policy = CTRL_POLICY_SYNC;
			} else {
				policy = CTRL_POLICY_ASYNC;
			}
		} else {
			policy = CTRL_POLICY_ASYNC;
		}
	}

	#ifdef _CTRL_ARCH_OPENCL_GPU_
	Ctrl_OpenCLGpu_AllocKernel(noclgpu);
	#endif // _CTRL_ARCH_OPENCL_GPU_
	#ifdef _CTRL_ARCH_FPGA_
	Ctrl_FPGA_AllocKernel(nfpga);
	#endif // _CTRL_ARCH_FPGA_

	// CREATE HOST TASK QUEUE
	p_ctrl_host_stream = (Ctrl_TaskQueue *)malloc(sizeof(Ctrl_TaskQueue));
	Ctrl_TaskQueue_Init(p_ctrl_host_stream);

	// CREATE CONTROLLERS
	for (int i = 0; i < ndevs; i++) {
		Ctrl_Create(devs[i].type, i, devs[i].args);
	}

	// FREE TEMP DATA STRUCTURES
	free(devs);
	free(buffer);
}

HitWeights Ctrl_ConfigWeights() {
	return ctrl_weights;
}

///@endcond
