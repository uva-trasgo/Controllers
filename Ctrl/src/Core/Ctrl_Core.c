///@cond INTERNAL
/**
 * @file Ctrl_Core.c
 * @brief Source code for core functions of Controllers.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "Core/Ctrl_Core.h"
#include <string.h>

/**
 * Macro describing whether there is CPU architecture support, for stringification.
 * @see CTRL_COMPILED_WITH_SUPPORT_FOR
 */
#ifdef _CTRL_ARCH_CPU_
#define _CTRL_SUPPORT_CPU_ ON
#else // !_CTRL_ARCH_CPU_
#define _CTRL_SUPPORT_CPU_ OFF
#endif // _CTRL_ARCH_CPU_

/**
 * Macro describing whether there is CUDA architecture support, for stringification.
 * @see CTRL_COMPILED_WITH_SUPPORT_FOR
 */
#ifdef _CTRL_ARCH_CUDA_
#define _CTRL_SUPPORT_CUDA_ ON
#else // !_CTRL_ARCH_CUDA_
#define _CTRL_SUPPORT_CUDA_ OFF
#endif // _CTRL_ARCH_CUDA_

/**
 * Macro describing whether there is HIP architecture support, for stringification.
 * @see CTRL_COMPILED_WITH_SUPPORT_FOR
 */
#ifdef _CTRL_ARCH_HIP_
#define _CTRL_SUPPORT_HIP_ ON
#else // !_CTRL_ARCH_HIP_
#define _CTRL_SUPPORT_HIP_ OFF
#endif // _CTRL_ARCH_HIP_

/**
 * Macro describing whether there is OPENCL_GPU architecture support, for stringification.
 * @see CTRL_COMPILED_WITH_SUPPORT_FOR
 */
#ifdef _CTRL_ARCH_OPENCL_GPU_
#define _CTRL_SUPPORT_OPENCL_GPU_ ON
#else // !_CTRL_ARCH_OPENCL_GPU_
#define _CTRL_SUPPORT_OPENCL_GPU_ OFF
#endif // _CTRL_ARCH_OPENCL_GPU_

/**
 * Macro describing whether there is FPGA architecture support, for stringification.
 * @see CTRL_COMPILED_WITH_SUPPORT_FOR
 */
#ifdef _CTRL_ARCH_FPGA_
#define _CTRL_SUPPORT_FPGA_ ON
#else // !_CTRL_ARCH_FPGA_
#define _CTRL_SUPPORT_FPGA_ OFF
#endif // _CTRL_ARCH_FPGA_

/**
 * Static string exporting the supported architectures in a Ctrl library instance.
 */
__attribute__((unused)) const char CTRL_COMPILED_WITH_SUPPORT_FOR[] = "CTRL_COMPILED_WITH_SUPPORT_FOR__CPU_" CTRL_MACRO_STRINGIFY(_CTRL_SUPPORT_CPU_) "_CUDA_" CTRL_MACRO_STRINGIFY(_CTRL_SUPPORT_CUDA_) "_HIP_" CTRL_MACRO_STRINGIFY(_CTRL_SUPPORT_HIP_) "_OPENCL_GPU_" CTRL_MACRO_STRINGIFY(_CTRL_SUPPORT_OPENCL_GPU_) "_FPGA_" CTRL_MACRO_STRINGIFY(_CTRL_SUPPORT_FPGA_);

/**
 * Optional weights for distributed computations.
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
 * Host memory alignment.
 */
int mem_alignment = 0;

/**
 * Path to the FPGA kernel binaries.
 */
char *Ctrl_FPGA_kernels_path;

/**
 * Policy to be used by ctrls.
 */
Ctrl_Policy policy = CTRL_POLICY_DEFAULT;

/**
 * Pointer to stream for host task execution, consumed by host task thread
 * @see Ctrl_Thread_HostTask, Ctrl_LaunchHostTask
 */
Ctrl_TaskQueue *p_ctrl_host_stream = NULL;

/**
 * Queue used to handle driver event destruction, consumed by queue manager thread
 * @see Ctrl_GenericEvent_Release, Ctrl_GenericEvent_Destroy, Ctrl_Thread_QueueManager
 */
Ctrl_TaskQueue *p_ctrl_event_destroy_queue = NULL;

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
 * Create and execute a task of type \p type that needs flags associated to \p p_ctrl .
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
 * Function for queue manager thread. Gets the host side queues used by ctrls and pushes tasks from them to driver streams.
 *
 * @see Ctrl_Thread_Init
 */
void Ctrl_Thread_QueueManager();

/**
 * Returns the number of threads needed for \p p_ctrl.
 *
 * @param p_ctrl Ctrl we want to get the number of extra threads of.
 *
 * @return Number of threads needed for \p p_ctrl.
 */
int Ctrl_GetNumThreads(Ctrl *p_ctrl);

/**
 * Returns the number of queues needed by \p p_ctrl.
 *
 * @param p_ctrl Ctrl we want to get the number of queues.
 *
 * @return Number of queues used by \p p_ctrl.
 */
int Ctrl_GetNumQueues(Ctrl *p_ctrl);

/**
 * Get the pointers to the host queues used by \p p_ctrl on list \p pp_queues
 *
 * @param p_ctrl Ctrl to get the queues from.
 * @param pp_queues [out] Pointer to pointers to the host queues used by this ctrl
 * @return Pointer right after the queue pointers stored on \p pp_queues
 *
 * @pre \p pp_queues must have enough memory allocated to store all queues from this ctrl.
 * @see Ctrl_GetNumQueues
 */
Ctrl_TaskQueue **Ctrl_GetHostQueues(Ctrl *p_ctrl, Ctrl_TaskQueue **pp_queues);

/**
 * Creates a ctrl with the config specified in \p dev
 *
 * @param dev Device specification. Corresponds to a line of the config file for the ctrl_block
 *
 * @see __ctrl_block__ Ctrl_ParseConfig
 */
void Ctrl_Create(Ctrl_Config_Dev dev);

/**
 * @brief Push a wait operation of \p event to \p stream if they are compatible
 *
 * This is an optimization because waiting for CUDA, HIP and OpenCL on the host via busy-wait testing is very expensive
 *
 * @param event Event to wait to
 * @param qid Queue index inside the ctrl to enqueue the wait into
 * @param p_ctrl Ctrl recieving \p event
 * @return true \p event was successfully enqueued into \p stream
 * @return false \p event is not compatible with \p stream
 */
bool Ctrl_PushEventIfCompat(Ctrl_GenericEvent event, int qid, Ctrl *p_ctrl);

void Ctrl_Init(int *pargc, char ***pargv) {
	hit_comInit(pargc, pargv);

	int    argc = *pargc;
	char **argv = *pargv;

	int i;
	for (i = 0; i < argc; i++) {
		if (!strncmp(argv[i], "--fpga-kernels-path=", 20))
			break;
	}
	// If the argument was not found, i is equal to argc.

	if (i < argc) {
		Ctrl_FPGA_kernels_path = (char *)malloc(strlen(&argv[i][20]) * sizeof(char));
		strcpy(Ctrl_FPGA_kernels_path, &argv[i][20]);

		// Fix argc and argv:
		for (; i < argc - 1; i++) {
			argv[i] = argv[i + 1];
		}
		argv[i] = NULL; // Erase last argument
		(*pargc)--;
	} else {
		// Argument not found. Use default path (same as the executed program's, argv[0]).

		// Extract directory path from program's path, find rightmost '/':
		int last_slash_idx = 0;
		for (int i = 0; i < strlen(argv[0]); i++) {
			if (argv[0][i] == '/')
				last_slash_idx = i;
		}

		Ctrl_FPGA_kernels_path                 = (char *)malloc((last_slash_idx + 1) * sizeof(char));
		Ctrl_FPGA_kernels_path[last_slash_idx] = '\0';
		strncpy(Ctrl_FPGA_kernels_path, argv[0], last_slash_idx);
	}
}

void Ctrl_PinToHostNuma() {
	hwloc_obj_t obj = hwloc_get_obj_by_type(topo, HWLOC_OBJ_NUMANODE, host_node);
	if (!obj) {
		fprintf(stderr, "[Ctrl_PinToHostNuma] Warning host NUMA node %d not found.\n", host_node);
		fflush(stderr);
		return;
	}
	if (hwloc_set_cpubind(topo, obj->cpuset, HWLOC_CPUBIND_THREAD) != 0) {
		fprintf(stderr, "[Ctrl_PinToHostNuma] Warning pinning thread to host numa node %d returned an error, make sure you have permission to use those resources.\n", host_node);
		fflush(stderr);
		return;
	}

	hwloc_cpuset_t allowed_numa_cpuset = hwloc_bitmap_alloc();
	hwloc_bitmap_and(allowed_numa_cpuset, obj->cpuset, hwloc_topology_get_allowed_cpuset(topo));
	int allowed_numa_cores = hwloc_get_nbobjs_inside_cpuset_by_type(topo, allowed_numa_cpuset, HWLOC_OBJ_CORE);
	hwloc_bitmap_free(allowed_numa_cpuset);

	static int bound_threads = 0;

	int n_thr;
	#pragma omp atomic capture
	n_thr = ++bound_threads;

	if (allowed_numa_cores < n_thr) {
		fprintf(stderr, "[Ctrl_PinToHostNuma] Warning only %d cores are allowed for use in host numa node %d but %d threads are currently bound to it.\n", allowed_numa_cores, host_node, bound_threads);
		fflush(stderr);
	}
}

int Ctrl_Thread_Init() {

	if (omp_get_thread_num() == 0) {
		// bind main thread to host numa node
		Ctrl_PinToHostNuma();
		return 0;
	} else if (omp_get_thread_num() == 1) { // host task executor
		// bind host task thread to host numa node
		Ctrl_PinToHostNuma();
		Ctrl_Thread_HostTask();
	} else if (omp_get_thread_num() == 2) { // queue manager
		// bind queue manager thread to host numa node
		Ctrl_PinToHostNuma();
		Ctrl_Thread_QueueManager();
	} else { // spawner threads
		Ctrl_Thread_Spawner();
	}

	// All threads ready for main thread to cleanup
	#pragma omp barrier

	return 1;
}

void Ctrl_Thread_HostTask() {
	// Explicitly initialize cuda runtime for this thread to avoid performace penalties
	#ifdef _CTRL_ARCH_CUDA_
	for (int i = 0; i < n_ctrls; i++) {
		if (p_ctrl_global[i].type == CTRL_TYPE_CUDA) {
			Ctrl_Cuda_SetDevice();
		}
	}
	#endif // _CTRL_ARCH_CUDA_

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
				Ctrl_Cpu_ThreadInit(&(p_ctrl->p_impl->cpu), topo);
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

void Ctrl_Create(Ctrl_Config_Dev dev) {
	static int id     = 0;
	PCtrl      p_ctrl = &p_ctrl_global[id];
	p_ctrl->p_impl    = (Ctrl_Impl *)malloc(sizeof(Ctrl_Impl));
	p_ctrl->id        = id++;
	p_ctrl->type      = dev.type;
	// TODO @sergioalo send id as param to inner create?
	switch (dev.type) {
			#ifdef _CTRL_ARCH_CPU_
		case CTRL_TYPE_CPU:
			p_ctrl->p_impl->cpu.topo      = topo;
			p_ctrl->p_impl->cpu.global_id = p_ctrl->id;
			Ctrl_Cpu_Create(&(p_ctrl->p_impl->cpu), policy, dev);
			break;
			#endif // _CTRL_ARCH_CPU_

			#ifdef _CTRL_ARCH_CUDA_
		case CTRL_TYPE_CUDA:
			p_ctrl->p_impl->cuda.global_id = p_ctrl->id;
			Ctrl_Cuda_Create(&(p_ctrl->p_impl->cuda), policy, dev);
			break;
			#endif // _CTRL_ARCH_CUDA_

			#ifdef _CTRL_ARCH_HIP_
		case CTRL_TYPE_HIP:
			p_ctrl->p_impl->hip.global_id = p_ctrl->id;
			Ctrl_Hip_Create(&(p_ctrl->p_impl->hip), policy, dev);
			break;
			#endif // _CTRL_ARCH_HIP_

			#ifdef _CTRL_ARCH_OPENCL_GPU_
		case CTRL_TYPE_OPENCL_GPU:
			p_ctrl->p_impl->opencl_gpu.global_id = p_ctrl->id;
			Ctrl_OpenCLGpu_Create(&(p_ctrl->p_impl->opencl_gpu), policy, dev);
			break;
			#endif // _CTRL_ARCH_OPENCL_GPU_

			#ifdef _CTRL_ARCH_FPGA_
		case CTRL_TYPE_FPGA:
			p_ctrl->p_impl->fpga.global_id = p_ctrl->id;
			Ctrl_FPGA_Create(&(p_ctrl->p_impl->fpga), policy, dev);
			break;
			#endif // _CTRL_ARCH_FPGA_
		default:
			fprintf(stderr, "[Ctrl_Create] Unknown or unsupported architecture: %d\n", dev.type);
			exit(EXIT_FAILURE);
	}
}

PCtrl Ctrl_Get(int id) {
	if (id < 0 || id >= Ctrl_GetNCtrls()) {
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
			fprintf(stderr, "[Ctrl_ExecTask] Unsupported architecture %d. Recompile Ctrl library with the proper support.\n", p_ctrl->type);
			exit(EXIT_FAILURE);
	}

	if (p_task->task_type != CTRL_TASK_TYPE_HOST && p_task->task_type != CTRL_TASK_TYPE_KERNEL) {
		Ctrl_TaskQueue_FreeTask(p_task);
	}
}

void Ctrl_EndBlock() {
	// Wait for all operations to end before starting cleanup
	Ctrl_Synchronize();

	for (int i = 0; i < Ctrl_GetNCtrls(); i++) {
		// Tell ctrls to cleanup their internal stuff and propagate the destroy signal
		Ctrl_AddTask(&p_ctrl_global[i], CTRL_TASK_TYPE_DESTROYCTRL, NULL);
		// Free internal part of the ctrl
		free(p_ctrl_global[i].p_impl);
		p_ctrl_global[i].p_impl = NULL;
	}

	// Send destroy signal to host task queue
	Ctrl_Task task = CTRL_TASK_NULL;
	task.task_type = CTRL_TASK_TYPE_DESTROYCTRL;
	Ctrl_TaskQueue_Push(p_ctrl_host_stream, task);
	// send destroy signal to queue manager thread
	if (p_ctrl_event_destroy_queue != NULL)
		Ctrl_TaskQueue_Push(p_ctrl_event_destroy_queue, task);

	// Wait for other threads to be ready for ctrl list full cleanup
	#pragma omp barrier
	free(p_ctrl_global);
	free(ctrl_weights.ratios);
	hwloc_topology_destroy(topo);
	free(Ctrl_FPGA_kernels_path);
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

void Ctrl_LaunchHostTask(Ctrl_Task task) {
	Ctrl_Task *p_task = &task;
	// wait for appropiate events from arguments according to their role
	for (int i = 0; i < p_task->n_arguments; i++) {
		if (p_task->p_roles[i] != KERNEL_INVAL) {
			HitTile   *p_tile      = (HitTile *)(p_task->pp_pointers[i]);
			Ctrl_Tile *p_tile_data = (Ctrl_Tile *)(p_tile->ext);

			if (p_tile->memStatus == HIT_MS_NOMEM) {
				fprintf(stderr, "[Ctrl_Core] Ctrl_LaunchHostTask: Launching host task %s with tile with no host memory as argument %d\n", p_task->p_func_name, i);
				fflush(stderr);
				exit(EXIT_FAILURE);
			}

			for (int j = 0; j < Ctrl_GetNCtrls(); j++) {
				Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[j];
				if (p_tile_data_impl->type == CTRL_TYPE_NULL) {
					continue;
				}

				// TODO @sergioalo Ctrl_MoveFromInner does unnecesary checks

				// TODO @sergioalo dependance mode should be on abstract ctrl
				// if tile's role is IN or IO, is not updated on host, and is updated on this device transfer it
				bool move_tile = p_task->p_roles[i] != KERNEL_OUT && p_tile_data->host_status == CTRL_TILE_INVALID && p_tile_data_impl->device_status == CTRL_TILE_VALID;

				switch (p_tile_data_impl->type) {
					#ifdef _CTRL_ARCH_CPU_
					case CTRL_TYPE_CPU:
						if (move_tile && p_ctrl_global[j].p_impl->cpu.dependance_mode == CTRL_MODE_IMPLICIT) {
							Ctrl_MoveFromInner(&p_ctrl_global[j], p_tile);
						}
						break;
					#endif // _CTRL_ARCH_CPU_

					#ifdef _CTRL_ARCH_CUDA_
					case CTRL_TYPE_CUDA:
						if (move_tile && p_ctrl_global[j].p_impl->cuda.dependance_mode == CTRL_MODE_IMPLICIT) {
							Ctrl_MoveFromInner(&p_ctrl_global[j], p_tile);
						}
						break;
					#endif // _CTRL_ARCH_CUDA_

					#ifdef _CTRL_ARCH_HIP_
					case CTRL_TYPE_HIP:
						if (move_tile && p_ctrl_global[j].p_impl->hip.dependance_mode == CTRL_MODE_IMPLICIT) {
							Ctrl_MoveFromInner(&p_ctrl_global[j], p_tile);
						}
						break;
					#endif // _CTRL_ARCH_HIP_

					#ifdef _CTRL_ARCH_OPENCL_GPU_
					case CTRL_TYPE_OPENCL_GPU:
						if (move_tile && p_ctrl_global[j].p_impl->opencl_gpu.dependance_mode == CTRL_MODE_IMPLICIT) {
							Ctrl_MoveFromInner(&p_ctrl_global[j], p_tile);
						}
						break;
					#endif // _CTRL_ARCH_OPENCL_GPU_

					#ifdef _CTRL_ARCH_FPGA_
					case CTRL_TYPE_FPGA:
						if (move_tile && p_ctrl_global[j].p_impl->fpga.dependance_mode == CTRL_MODE_IMPLICIT) {
							Ctrl_MoveFromInner(&p_ctrl_global[j], p_tile);
						}
						break;
					#endif // _CTRL_ARCH_FPGA_
					default:
						fprintf(stderr, "[Ctrl_Core] Ctrl_LaunchHostTask: unknown arch tile %d on host task %s\n", p_tile_data_impl->type, p_task->p_func_name);
						exit(EXIT_FAILURE);
				}
				Ctrl_HostTaskWait(p_tile_data_impl, p_task->p_roles[i], p_ctrl_host_stream);
			}

			if (p_task->p_roles[i] != KERNEL_OUT && p_tile_data->host_status == CTRL_TILE_INVALID) {
				fprintf(stderr, "[Ctrl_Core] Warning: Tile %d with uninitialized data as input on host task %s\n", i, p_task->p_func_name);
				fflush(stderr);
			}

			Ctrl_GenericEvent_StreamWait(p_tile_data->last_host_write_event, p_ctrl_host_stream);
			if (p_task->p_roles[i] != KERNEL_IN) {
				for (int j = 0; j < Ctrl_GetNCtrls(); j++) {
					Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[j];
					if (p_tile_data_impl->type == CTRL_TYPE_NULL) {
						continue;
					}
					if (p_tile_data_impl->device_status == CTRL_TILE_VALID) p_tile_data_impl->device_status = CTRL_TILE_INVALID;
				}

				p_tile_data->host_status = CTRL_TILE_VALID;
				Ctrl_GenericEvent_StreamWait(p_tile_data->last_host_read_event, p_ctrl_host_stream);
			}
		}
	}

	if (policy == CTRL_POLICY_SYNC) {
		Ctrl_SyncWait(p_ctrl_host_stream);
	}

	Ctrl_TaskQueue_Push(p_ctrl_host_stream, *p_task);

	// update events on arguments according to their roles
	for (int i = 0; i < p_task->n_arguments; i++) {
		if (p_task->p_roles[i] != KERNEL_INVAL) {
			HitTile   *p_tile      = (HitTile *)(p_task->pp_pointers[i]);
			Ctrl_Tile *p_tile_data = (Ctrl_Tile *)(p_tile->ext);

			if (p_task->p_roles[i] != KERNEL_IN) {
				Ctrl_CpuEvent_Record(&p_tile_data->last_host_write_event.event.event_cpu, p_ctrl_host_stream);
			}

			if (p_task->p_roles[i] != KERNEL_OUT) {
				Ctrl_CpuEvent_Record(&p_tile_data->last_host_read_event.event.event_cpu, p_ctrl_host_stream);
			}
		}
	}

	// wait for host task to finish if policy is sync
	if (policy == CTRL_POLICY_SYNC) {
		Ctrl_TaskQueue_Synchronize(p_ctrl_host_stream);
	}
}

void Ctrl_GlobalSync(Ctrl *p_ctrl) {
	Ctrl_AddTask(p_ctrl, CTRL_TASK_TYPE_GLOBALSYNC, NULL);
}

void Ctrl_AllocInner(Ctrl *p_ctrl, HitTile *p_tile, int flags) {
	// Allocate host memory
	if (p_tile->memStatus != HIT_MS_OWNER && (flags & CTRL_MEM_ALLOC_HOST || !(flags & CTRL_MEM_ALLOC_DEV))) {
		Ctrl_Tile *p_tile_data = (Ctrl_Tile *)(p_tile->ext);
		size_t     alignment   = (flags & CTRL_MEM_ALIGNED) && hit_tileDims(*p_tile) > 1 ? mem_alignment : 0;
		if (alignment % p_tile->baseExtent != 0) {
			fprintf(stderr, "[Ctrl_Alloc] Error: tile host alignment %lu is not divisible by type size %lu.\n", alignment, p_tile->baseExtent);
			exit(EXIT_FAILURE);
		}

		Ctrl_Tile_UpdateOrigAcumCards(p_tile->origAcumCard, hit_tileDims(*p_tile), p_tile->card, alignment / p_tile->baseExtent);
		bool pinned = false;
		// Allocate host "pinned" memory
		switch (p_ctrl->type) {
			#ifdef _CTRL_ARCH_CPU_
			case CTRL_TYPE_CPU:
				// no pinned memory on cpu
				break;
			#endif // _CTRL_ARCH_CPU_

			#ifdef _CTRL_ARCH_CUDA_
			case CTRL_TYPE_CUDA:
				pinned = Ctrl_Cuda_AllocPinned(&p_ctrl->p_impl->cuda, p_tile, flags);
				break;
			#endif // _CTRL_ARCH_CUDA_

			#ifdef _CTRL_ARCH_HIP_
			case CTRL_TYPE_HIP:
				pinned = Ctrl_Hip_AllocPinned(&p_ctrl->p_impl->hip, p_tile, flags);
				break;
			#endif // _CTRL_ARCH_HIP_

			#ifdef _CTRL_ARCH_OPENCL_GPU_
			case CTRL_TYPE_OPENCL_GPU:
				pinned = Ctrl_OpenCLGpu_AllocPinned(&p_ctrl->p_impl->opencl_gpu, p_tile, flags);
				break;
			#endif // _CTRL_ARCH_OPENCL_GPU_

			#ifdef _CTRL_ARCH_FPGA_
			case CTRL_TYPE_FPGA:
				pinned = Ctrl_FPGA_AllocPinned(&p_ctrl->p_impl->fpga, p_tile, flags);
				break;
			#endif // _CTRL_ARCH_FPGA_
			default:
				fprintf(stderr, "[Ctrl_Core] Ctrl_AllocPinned: Unknown ctrl type %d", p_ctrl->type);
				exit(EXIT_FAILURE);
		}
		if (!pinned) {
			// Allocate host memory the usual way
			p_tile_data->pinned = CTRL_TYPE_NULL;
			p_tile->data        = (void *)malloc((size_t)p_tile->origAcumCard[0] * p_tile->baseExtent);
		}
		p_tile_data->host_status = CTRL_TILE_INVALID;
		p_tile->memPtr           = p_tile->data;
		p_tile->memStatus        = HIT_MS_OWNER;
	}

	Ctrl_AddTaskFlagged(p_ctrl, CTRL_TASK_TYPE_ALLOCTILE, p_tile, flags);
}

void Ctrl_CreateTexInner(Ctrl *p_ctrl, HitTile *p_tile, Ctrl_TexDesc tex_desc) {
	switch (p_ctrl->type) {
		#ifdef _CTRL_ARCH_CPU_
		case CTRL_TYPE_CPU:
			Ctrl_Cpu_CreateTex(&p_ctrl->p_impl->cpu, p_tile, tex_desc);
			break;
		#endif // _CTRL_ARCH_CPU_

		#ifdef _CTRL_ARCH_CUDA_
		case CTRL_TYPE_CUDA:
			Ctrl_Cuda_CreateTex(&p_ctrl->p_impl->cuda, p_tile, tex_desc);
			break;
		#endif // _CTRL_ARCH_CUDA_

		#ifdef _CTRL_ARCH_HIP_
		case CTRL_TYPE_HIP:
			Ctrl_Hip_CreateTex(&p_ctrl->p_impl->hip, p_tile, tex_desc);
			break;
		#endif // _CTRL_ARCH_HIP_

		#ifdef _CTRL_ARCH_OPENCL_GPU_
		case CTRL_TYPE_OPENCL_GPU:
			Ctrl_OpenCLGpu_CreateTex(&p_ctrl->p_impl->opencl_gpu, p_tile, tex_desc);
			break;
		#endif // _CTRL_ARCH_OPENCL_GPU_

		#ifdef _CTRL_ARCH_FPGA_
		case CTRL_TYPE_FPGA:
			Ctrl_FPGA_CreateTex(&p_ctrl->p_impl->fpga, p_tile, tex_desc);
			break;
		#endif // _CTRL_ARCH_FPGA_
		default:
			fprintf(stderr, "[Ctrl_Core] Ctrl_CreateTex: Unknown ctrl type %d", p_ctrl->type);
			exit(EXIT_FAILURE);
	}
}

void *Ctrl_GetDevPtrInner(Ctrl *p_ctrl, HitTile *p_tile) {
	if (hit_tileIsNull(*p_tile) || p_tile->ext == NULL)
		return NULL;

	switch (p_ctrl->type) {
		#ifdef _CTRL_ARCH_CPU_
		case CTRL_TYPE_CPU:
			return Ctrl_Cpu_GetDevPtr(&p_ctrl->p_impl->cpu, p_tile);
		#endif // _CTRL_ARCH_CPU_

		#ifdef _CTRL_ARCH_CUDA_
		case CTRL_TYPE_CUDA:
			return Ctrl_Cuda_GetDevPtr(&p_ctrl->p_impl->cuda, p_tile);
		#endif // _CTRL_ARCH_CUDA_

		#ifdef _CTRL_ARCH_HIP_
		case CTRL_TYPE_HIP:
			return Ctrl_Hip_GetDevPtr(&p_ctrl->p_impl->hip, p_tile);
		#endif // _CTRL_ARCH_HIP_
		case CTRL_TYPE_OPENCL_GPU:
		case CTRL_TYPE_FPGA:
			return NULL;
		default:
			fprintf(stderr, "[Ctrl_Core] Ctrl_GetDevPtr: Unknown ctrl type %d", p_ctrl->type);
			exit(EXIT_FAILURE);
	}
}

void Ctrl_SelectInner(HitTile *p_tile, int flags) {
	// @arturo: Bug, select tasks for NULL Tiles should not be introduced in the queue
	if (hit_tileIsNull(*p_tile)) {
		#ifdef _CTRL_DEBUG_
		fprintf(stderr, "Warning: NULL tile on Ctrl_SelectInner\n");
		fflush(stderr);
		#endif // _CTRL_DEBUG_
		return;
	}

	Ctrl_DomainInner(p_tile);

	Ctrl_Tile *p_parent_data = ((Ctrl_Tile *)(p_tile->ref->ext));

	if (p_tile->memStatus == HIT_MS_NOT_OWNER) {
		Ctrl_Tile *p_tile_data   = (Ctrl_Tile *)(p_tile->ext);
		p_tile_data->host_status = p_parent_data->host_status;
	}

	for (int i = 0; i < n_ctrls; i++) {
		Ctrl_Tile_Impl *p_parent_data_impl = &p_parent_data->p_impls[i];
		// check if parent is asociated with ctrl i
		if (p_parent_data_impl->type == CTRL_TYPE_NULL) continue;

		Ctrl_AddTaskFlagged(&p_ctrl_global[i], CTRL_TASK_TYPE_SELECTTILE, p_tile, flags);
	}
}

void Ctrl_DomainInner(HitTile *p_tile) {
	Ctrl_Tile *p_tile_data = (Ctrl_Tile *)malloc(sizeof(Ctrl_Tile));

	p_tile_data->p_impls = (Ctrl_Tile_Impl *)calloc(Ctrl_GetNCtrls(), sizeof(Ctrl_Tile_Impl));

	p_tile_data->host_status = CTRL_TILE_INVALID;
	p_tile_data->valid_impls = 0;

	p_tile_data->last_host_read_event  = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_CPU, -1);
	p_tile_data->last_host_write_event = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_CPU, -1);

	p_tile->ext = (void *)p_tile_data;
}

void Ctrl_FreeInner(Ctrl *p_ctrl, HitTile *p_tile) {
	if (hit_tileIsNull(*p_tile)) {
		#ifdef _CTRL_DEBUG_
		fprintf(stderr, "Warning: NULL tile on Ctrl_FreeInner\n");
		fflush(stderr);
		#endif // _CTRL_DEBUG_
		return;
	}

	if (p_ctrl == NULL) {
		Ctrl_Tile *p_tile_data = (Ctrl_Tile *)p_tile->ext;
		for (int i = 0; i < n_ctrls; i++) {
			// check if parent is asociated with ctrl i
			if (p_tile_data->p_impls[i].type == CTRL_TYPE_NULL) continue;

			Ctrl_AddTask(&p_ctrl_global[i], CTRL_TASK_TYPE_FREETILE, p_tile);
		}
	} else {
		Ctrl_AddTask(p_ctrl, CTRL_TASK_TYPE_FREETILE, p_tile);
	}
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
	for (int i = 0; i < Ctrl_GetNCtrls(); i++) {
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
					} else if (CTRL_IMPL_IN_RANGE(avail_impls[i], CUDALIB)) {
						switch (avail_impls[i]) {
							case CUDALIB_DEFAULT:
								result = avail_impls[i];
								break;

							#ifdef _CTRL_CUBLAS_
							case CUDALIB_CUBLAS:
								result = avail_impls[i];
								break;
							#endif // _CTRL_CUBLAS_

							#ifdef _CTRL_MAGMA_
							case CUDALIB_MAGMA:
								result = avail_impls[i];
								break;
							#endif // _CTRL_MAGMA_
							default:
								#ifdef _CTRL_DEBUG_
								fprintf(stderr, "Warning: ignoring kernel implementation of type %d. Consider recompiling with proper support\n", avail_impls[i]);
								fflush(stderr);
								#endif // _CTRL_DEBUG_
								break;
						}
					}
				}
			}
			break;
		case CTRL_TYPE_HIP:
			for (int i = 0; i < n_impl; i++) {
				if (avail_impls[i] > result) {
					if (CTRL_IMPL_IN_RANGE(avail_impls[i], GENERIC) || CTRL_IMPL_IN_RANGE(avail_impls[i], HIP)) {
						result = avail_impls[i];
					} else if (CTRL_IMPL_IN_RANGE(avail_impls[i], HIPLIB)) {
						switch (avail_impls[i]) {
							case HIPLIB_DEFAULT:
								result = avail_impls[i];
								break;

							#ifdef _CTRL_HIPBLAS_
							case HIPLIB_HIPBLAS:
								result = avail_impls[i];
								break;
							#endif // _CTRL_HIPBLAS_
							default:
								#ifdef _CTRL_DEBUG_
								fprintf(stderr, "Warning: ignoring kernel implementation of type %d. Consider recompiling with proper support\n", avail_impls[i]);
								#endif // _CTRL_DEBUG_
								break;
						}
					}
				}
			}
			break;
		case CTRL_TYPE_OPENCL_GPU:
			for (int i = 0; i < n_impl; i++) {
				if (avail_impls[i] > result) {
					if (CTRL_IMPL_IN_RANGE(avail_impls[i], GENERIC) || CTRL_IMPL_IN_RANGE(avail_impls[i], OPENCLGPU) || CTRL_IMPL_IN_RANGE(avail_impls[i], OPENCLGPULIB)) {
						result = avail_impls[i];
					}
				}
			}
			break;
		case CTRL_TYPE_FPGA:
			for (int i = 0; i < n_impl; i++) {
				if (avail_impls[i] > result) {
					if (CTRL_IMPL_IN_RANGE(avail_impls[i], FPGA) || CTRL_IMPL_IN_RANGE(avail_impls[i], FPGALIB)) {
						result = avail_impls[i];
					}
				}
			}
			break;
		case CTRL_TYPE_NULL:
			fprintf(stderr, "[Ctrl_Core] Ctrl_Dev: null ctrl type\n");
			exit(EXIT_FAILURE);
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
			Ctrl_GenericEvent_Release(p_task->event);
			break;
		case CTRL_TASK_TYPE_SIGNALEVENT:
			Ctrl_GenericEvent_Signal(p_task->event);
			break;
		default:
			fprintf(stderr, "[Core EvalTask] Task type not implemented on this stream\n");
			exit(EXIT_FAILURE);
			break;
	}
}

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

int Ctrl_GetNumQueues(Ctrl *p_ctrl) {
	switch (p_ctrl->type) {
		#ifdef _CTRL_ARCH_CPU_
		case CTRL_TYPE_CPU:
			return Ctrl_Cpu_GetNumQueues(&(p_ctrl->p_impl->cpu));
		#endif // _CTRL_ARCH_CPU_

		#ifdef _CTRL_ARCH_CUDA_
		case CTRL_TYPE_CUDA:
			return Ctrl_Cuda_GetNumQueues(&(p_ctrl->p_impl->cuda));
		#endif // _CTRL_ARCH_CUDA_

		#ifdef _CTRL_ARCH_HIP_
		case CTRL_TYPE_HIP:
			return Ctrl_Hip_GetNumQueues(&(p_ctrl->p_impl->hip));
		#endif // _CTRL_ARCH_HIP_

		#ifdef _CTRL_ARCH_OPENCL_GPU_
		case CTRL_TYPE_OPENCL_GPU:
			return Ctrl_OpenCLGpu_GetNumQueues(&(p_ctrl->p_impl->opencl_gpu));
		#endif // _CTRL_ARCH_OPENCL_GPU_

		#ifdef _CTRL_ARCH_FPGA_
		case CTRL_TYPE_FPGA:
			return Ctrl_FPGA_GetNumQueues(&(p_ctrl->p_impl->fpga));
		#endif // _CTRL_ARCH_FPGA_
		default:
			fprintf(stderr, "[Ctrl_GetNumQueues] Unsupported architecture %d. Recompile Ctrl library with the proper support.\n", p_ctrl->type);
			exit(EXIT_FAILURE);
	}
}

Ctrl_TaskQueue **Ctrl_GetHostQueues(Ctrl *p_ctrl, Ctrl_TaskQueue **pp_queues) {
	switch (p_ctrl->type) {
		#ifdef _CTRL_ARCH_CPU_
		case CTRL_TYPE_CPU:
			return Ctrl_Cpu_GetHostQueues(&(p_ctrl->p_impl->cpu), pp_queues);
		#endif // _CTRL_ARCH_CPU_

		#ifdef _CTRL_ARCH_CUDA_
		case CTRL_TYPE_CUDA:
			return Ctrl_Cuda_GetHostQueues(&(p_ctrl->p_impl->cuda), pp_queues);
		#endif // _CTRL_ARCH_CUDA_

		#ifdef _CTRL_ARCH_HIP_
		case CTRL_TYPE_HIP:
			return Ctrl_Hip_GetHostQueues(&(p_ctrl->p_impl->hip), pp_queues);
		#endif // _CTRL_ARCH_HIP_

		#ifdef _CTRL_ARCH_OPENCL_GPU_
		case CTRL_TYPE_OPENCL_GPU:
			return Ctrl_OpenCLGpu_GetHostQueues(&(p_ctrl->p_impl->opencl_gpu), pp_queues);
		#endif // _CTRL_ARCH_OPENCL_GPU_

		#ifdef _CTRL_ARCH_FPGA_
		case CTRL_TYPE_FPGA:
			return Ctrl_FPGA_GetHostQueues(&(p_ctrl->p_impl->fpga), pp_queues);
		#endif // _CTRL_ARCH_FPGA_
		default:
			fprintf(stderr, "[Ctrl_GetHostQueues] Unsupported architecture. Recompile Ctrl library with the proper support.\n");
			exit(EXIT_FAILURE);
	}
}

void Ctrl_SetPolicy(Ctrl_Policy p) {
	policy = p;
}

void Ctrl_InitWeights(float com_weight) {
	// Get number of active processes to create array
	HitTopology com_topo         = hit_topology(plug_topPlain);
	int         global_num_procs = hit_topDimCard(com_topo, 0);
	int         global_rank      = hit_topDimRank(com_topo, 0);
	ctrl_weights.ratios          = (float *)malloc(sizeof(float) * global_num_procs);
	for (int i = 0; i < global_num_procs; i++)
		ctrl_weights.ratios[global_rank] = 1.0f; // Default value
	ctrl_weights.num_procs = global_num_procs;
	hit_topFree(com_topo);

	// Communicate weights
	MPI_Allgather(&com_weight, 1, MPI_FLOAT, ctrl_weights.ratios, 1, MPI_FLOAT, MPI_COMM_WORLD);
}

HitWeights Ctrl_GetWeights() {
	return ctrl_weights;
}

void Ctrl_InitCore(Ctrl_Config cfg) {
	Ctrl_InitWeights(cfg.weight);
	host_node     = cfg.host_affinity;
	mem_alignment = cfg.host_alignment != 0 ? cfg.host_alignment : CTRL_DEFAULT_TILE_ALIGNMENT;

	// Load hardware topology info
	hwloc_topology_init(&topo);
	hwloc_topology_set_flags(topo, HWLOC_TOPOLOGY_FLAG_INCLUDE_DISALLOWED);
	hwloc_topology_load(topo);

	// Set policy
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

	// Count ctrls
	int __attribute__((unused)) ncpu = 0, nocl = 0, nfpga = 0, real_ndevs = 0;
	for (int i = 0; i < cfg.ndevs; i++) {
		if (cfg.p_devs[i].type == CTRL_TYPE_NULL)
			continue;
		real_ndevs++;
		switch (cfg.p_devs[i].type) {
			case CTRL_TYPE_CPU: ncpu++; break;
			case CTRL_TYPE_OPENCL_GPU: nocl++; break;
			case CTRL_TYPE_FPGA: nfpga++; break;
			default: break;
		}
	}

	// Initialize controllers list
	n_ctrls       = real_ndevs;
	p_ctrl_global = (Ctrl *)malloc(n_ctrls * sizeof(Ctrl));
	omp_set_num_threads(3 + ncpu);

	#ifdef _CTRL_ARCH_OPENCL_GPU_
	Ctrl_OpenCLGpu_AllocKernel(nocl);
	#endif // _CTRL_ARCH_OPENCL_GPU_
	#ifdef _CTRL_ARCH_FPGA_
	Ctrl_FPGA_AllocKernel(nfpga);
	#endif // _CTRL_ARCH_FPGA_

	// Create host task queue
	p_ctrl_host_stream = Ctrl_TaskQueue_Create();

	// Create controllers
	for (int i = 0; i < cfg.ndevs; i++) {
		if (cfg.p_devs[i].type == CTRL_TYPE_NULL)
			continue;

		Ctrl_Create(cfg.p_devs[i]);
	}
}

bool Ctrl_PushEventIfCompat(Ctrl_GenericEvent event, int qid, Ctrl *p_ctrl) {
	if (!Ctrl_Event_CheckCompat(event, p_ctrl->type, p_ctrl->id))
		return false;

	Ctrl_Task task = CTRL_TASK_NULL;
	task.task_type = CTRL_TASK_TYPE_WAITEVENT;
	task.stream    = qid;
	task.event     = event;

	switch (p_ctrl->type) {
		#ifdef _CTRL_ARCH_CUDA_
		case CTRL_TYPE_CUDA:
			Ctrl_Cuda_ExecTask(&task, &p_ctrl->p_impl->cuda);
			break;
		#endif // _CTRL_ARCH_CUDA_

		#ifdef _CTRL_ARCH_HIP_
		case CTRL_TYPE_HIP:
			Ctrl_Hip_ExecTask(&task, &p_ctrl->p_impl->hip);
			break;
		#endif // _CTRL_ARCH_HIP_

		#ifdef _CTRL_ARCH_OPENCL_GPU_
		case CTRL_TYPE_OPENCL_GPU:
			Ctrl_OpenCLGpu_ExecTask(&task, &p_ctrl->p_impl->opencl_gpu);
			break;
		#endif // _CTRL_ARCH_OPENCL_GPU_

		#ifdef _CTRL_ARCH_FPGA_
		case CTRL_TYPE_FPGA:
			Ctrl_FPGA_ExecTask(&task, &p_ctrl->p_impl->fpga);
			break;
		#endif // _CTRL_ARCH_FPGA_
		default:
			fprintf(stderr, "[Ctrl Core] Queue manager: Invalid ctrl type %d, task type %d (wait event push)\n", p_ctrl->type, task.task_type);
			exit(EXIT_FAILURE);
			break;
	}
	return true;
}

void Ctrl_Thread_QueueManager() {
	int n_queues = 0;
	int p_nqueues[n_ctrls];
	int p_displacements[n_ctrls];
	for (int cid = 0; cid < n_ctrls; cid++) {
		p_nqueues[cid] = Ctrl_GetNumQueues(&p_ctrl_global[cid]);
		n_queues += p_nqueues[cid];
		p_displacements[cid] = cid == 0 ? 0 : p_displacements[cid - 1] + p_nqueues[cid - 1];
	}

	if (n_queues == 0) {
		return;
	}

	// Extra queue for event destruction
	n_queues++;
	p_nqueues[n_ctrls - 1]++;

	Ctrl_TaskQueue  *pp_queues[n_queues];
	Ctrl_TaskQueue **pp_aux = pp_queues;
	for (int i = 0; i < n_ctrls; i++) {
		pp_aux = Ctrl_GetHostQueues(&p_ctrl_global[i], pp_aux);
	}
	p_ctrl_event_destroy_queue = Ctrl_TaskQueue_Create();
	pp_queues[n_queues - 1]    = p_ctrl_event_destroy_queue;

	while (true) {
		for (int cid = 0; cid < n_ctrls; cid++) {
			for (int qid = 0; qid < p_nqueues[cid]; qid++) {
				// global qid
				int gqid = p_displacements[cid] + qid;

				Ctrl_Task *p_task = Ctrl_TaskQueue_GetNext(pp_queues[gqid]);
				if (p_task == NULL) {
					continue;
				}

				Ctrl *p_ctrl = &p_ctrl_global[cid];

				switch (p_task->task_type) {
					case CTRL_TASK_TYPE_KERNEL:
					case CTRL_TASK_TYPE_MOVEFROM:
					case CTRL_TASK_TYPE_MOVETO:
						p_task->stream = qid;
						Ctrl_TaskQueue_Pop(pp_queues[gqid]);
						switch (p_ctrl->type) {
							#ifdef _CTRL_ARCH_CUDA_
							case CTRL_TYPE_CUDA:
								Ctrl_Cuda_ExecTask(p_task, &p_ctrl->p_impl->cuda);
								break;
							#endif // _CTRL_ARCH_CUDA_

							#ifdef _CTRL_ARCH_HIP_
							case CTRL_TYPE_HIP:
								Ctrl_Hip_ExecTask(p_task, &p_ctrl->p_impl->hip);
								break;
							#endif // _CTRL_ARCH_HIP_

							#ifdef _CTRL_ARCH_OPENCL_GPU_
							case CTRL_TYPE_OPENCL_GPU:
								Ctrl_OpenCLGpu_ExecTask(p_task, &p_ctrl->p_impl->opencl_gpu);
								break;
							#endif // _CTRL_ARCH_OPENCL_GPU_

							#ifdef _CTRL_ARCH_FPGA_
							case CTRL_TYPE_FPGA:
								Ctrl_FPGA_ExecTask(p_task, &p_ctrl->p_impl->fpga);
								break;
							#endif // _CTRL_ARCH_FPGA_
							default:
								fprintf(stderr, "[Ctrl Core] Queue manager: Unknown ctrl type %d, task type %d\n", p_ctrl->type, p_task->task_type);
								exit(EXIT_FAILURE);
								break;
						}
						#pragma omp atomic update
						pp_queues[gqid]->last_finished++;
						break;
					case CTRL_TASK_TYPE_WAITEVENT:
						// use short circuit to push the event to driver stream if possible instead of waiting on the host
						if (Ctrl_PushEventIfCompat(p_task->event, qid, p_ctrl) || Ctrl_GenericEvent_Test(p_task->event)) {
							Ctrl_TaskQueue_Pop(pp_queues[gqid]);
							Ctrl_GenericEvent_Release(p_task->event);
							#pragma omp atomic update
							pp_queues[gqid]->last_finished++;
						}
						break;
					case CTRL_TASK_TYPE_DESTROYEVENT:
						// All driver events are destroyed here by this thread to avoid locks
						Ctrl_TaskQueue_Pop(pp_queues[gqid]);
						Ctrl_GenericEvent_Destroy(p_task->event);
						break;
					case CTRL_TASK_TYPE_DESTROYCTRL:
						for (int j = 0; j < n_queues; j++) {
							Ctrl_TaskQueue_Destroy(pp_queues[j]);
						}
						return;
					default:
						#ifdef _CTRL_DEBUG_
						fprintf(stderr, "[Ctrl Core] Queue manager: Unknown task type %d on queue %d of ctrl %d\n", p_task->task_type, qid, cid);
						fflush(stderr);
						#endif // _CTRL_DEBUG_

						// FIXME Sometimes a null task (type 0) arrives here, caused by optimization to destroy all events from the same thread. Likely from queue sync issues.
						// exit(EXIT_FAILURE);
						break;
				}
			}
		}
	}
}

void Ctrl_SyncWait(Ctrl_TaskQueue *p_queue) {
	if (policy == CTRL_POLICY_SYNC) {
		for (int i = 0; i < Ctrl_GetNCtrls(); i++) {
			switch (p_ctrl_global[i].type) {
				#ifdef _CTRL_ARCH_CPU_
				case CTRL_TYPE_CPU:
					Ctrl_Cpu_SyncWait(&p_ctrl_global[i].p_impl->cpu, p_queue);
					break;
				#endif // _CTRL_ARCH_CPU_

				#ifdef _CTRL_ARCH_CUDA_
				case CTRL_TYPE_CUDA:
					Ctrl_Cuda_SyncWait(&p_ctrl_global[i].p_impl->cuda, p_queue);
					break;
				#endif // _CTRL_ARCH_CUDA_

				#ifdef _CTRL_ARCH_HIP_
				case CTRL_TYPE_HIP:
					Ctrl_Hip_SyncWait(&p_ctrl_global[i].p_impl->hip, p_queue);
					break;
				#endif // _CTRL_ARCH_HIP_

				#ifdef _CTRL_ARCH_OPENCL_GPU_
				case CTRL_TYPE_OPENCL_GPU:
					Ctrl_OpenCLGpu_SyncWait(&p_ctrl_global[i].p_impl->opencl_gpu, p_queue);
					break;
				#endif // _CTRL_ARCH_OPENCL_GPU_

				#ifdef _CTRL_ARCH_FPGA_
				case CTRL_TYPE_FPGA:
					Ctrl_FPGA_SyncWait(&p_ctrl_global[i].p_impl->fpga, p_queue);
					break;
				#endif // _CTRL_ARCH_FPGA_
				default:
					fprintf(stderr, "[Ctrl_Core] Ctrl_SyncWait: unknown ctrl type %d", p_ctrl_global[i].type);
					exit(EXIT_FAILURE);
			}
		}
	}
}

void Ctrl_MoveToWait(Ctrl_Tile_Impl *p_tile_impl, Ctrl_TaskQueue *p_queue) {
	switch (p_tile_impl->type) {
		#ifdef _CTRL_ARCH_CPU_
		case CTRL_TYPE_CPU:
			Ctrl_Cpu_MoveToWait(p_tile_impl->tile.p_cpu, p_queue);
			break;
		#endif // _CTRL_ARCH_CPU_

		#ifdef _CTRL_ARCH_CUDA_
		case CTRL_TYPE_CUDA:
			Ctrl_Cuda_MoveToWait(p_tile_impl->tile.p_cuda, p_queue);
			break;
		#endif // _CTRL_ARCH_CUDA_

		#ifdef _CTRL_ARCH_HIP_
		case CTRL_TYPE_HIP:
			Ctrl_Hip_MoveToWait(p_tile_impl->tile.p_hip, p_queue);
			break;
		#endif // _CTRL_ARCH_HIP_

		#ifdef _CTRL_ARCH_OPENCL_GPU_
		case CTRL_TYPE_OPENCL_GPU:
			Ctrl_OpenCLGpu_MoveToWait(p_tile_impl->tile.p_opencl, p_queue);
			break;
		#endif // _CTRL_ARCH_OPENCL_GPU_

		#ifdef _CTRL_ARCH_FPGA_
		case CTRL_TYPE_FPGA:
			Ctrl_FPGA_MoveToWait(p_tile_impl->tile.p_fpga, p_queue);
			break;
		#endif // _CTRL_ARCH_FPGA_
		default:
			break;
	}
}

void Ctrl_MoveFromWait(Ctrl_Tile_Impl *p_tile_impl, Ctrl_TaskQueue *p_queue) {
	switch (p_tile_impl->type) {
		#ifdef _CTRL_ARCH_CPU_
		case CTRL_TYPE_CPU:
			Ctrl_Cpu_MoveFromWait(p_tile_impl->tile.p_cpu, p_queue);
			break;
		#endif // _CTRL_ARCH_CPU_

		#ifdef _CTRL_ARCH_CUDA_
		case CTRL_TYPE_CUDA:
			Ctrl_Cuda_MoveFromWait(p_tile_impl->tile.p_cuda, p_queue);
			break;
		#endif // _CTRL_ARCH_CUDA_

		#ifdef _CTRL_ARCH_HIP_
		case CTRL_TYPE_HIP:
			Ctrl_Hip_MoveFromWait(p_tile_impl->tile.p_hip, p_queue);
			break;
		#endif // _CTRL_ARCH_HIP_

		#ifdef _CTRL_ARCH_OPENCL_GPU_
		case CTRL_TYPE_OPENCL_GPU:
			Ctrl_OpenCLGpu_MoveFromWait(p_tile_impl->tile.p_opencl, p_queue);
			break;
		#endif // _CTRL_ARCH_OPENCL_GPU_

		#ifdef _CTRL_ARCH_FPGA_
		case CTRL_TYPE_FPGA:
			Ctrl_FPGA_MoveFromWait(p_tile_impl->tile.p_fpga, p_queue);
			break;
		#endif // _CTRL_ARCH_FPGA_
		default:
			break;
	}
}

void Ctrl_HostTaskWait(Ctrl_Tile_Impl *p_tile_impl, char rol, Ctrl_TaskQueue *p_queue) {
	switch (p_tile_impl->type) {
		#ifdef _CTRL_ARCH_CPU_
		case CTRL_TYPE_CPU:
			Ctrl_Cpu_HostTaskWait(p_tile_impl->tile.p_cpu, rol, p_queue);
			break;
		#endif // _CTRL_ARCH_CPU_

		#ifdef _CTRL_ARCH_CUDA_
		case CTRL_TYPE_CUDA:
			Ctrl_Cuda_HostTaskWait(p_tile_impl->tile.p_cuda, rol, p_queue);
			break;
		#endif // _CTRL_ARCH_CUDA_

		#ifdef _CTRL_ARCH_HIP_
		case CTRL_TYPE_HIP:
			Ctrl_Hip_HostTaskWait(p_tile_impl->tile.p_hip, rol, p_queue);
			break;
		#endif // _CTRL_ARCH_HIP_

		#ifdef _CTRL_ARCH_OPENCL_GPU_
		case CTRL_TYPE_OPENCL_GPU:
			Ctrl_OpenCLGpu_HostTaskWait(p_tile_impl->tile.p_opencl, rol, p_queue);
			break;
		#endif // _CTRL_ARCH_OPENCL_GPU_

		#ifdef _CTRL_ARCH_FPGA_
		case CTRL_TYPE_FPGA:
			Ctrl_FPGA_HostTaskWait(p_tile_impl->tile.p_fpga, rol, p_queue);
			break;
		#endif // _CTRL_ARCH_FPGA_
		default:
			break;
	}
}

void Ctrl_FreeHostInner(HitTile *p_tile) {
	Ctrl_Tile *p_tile_data = (Ctrl_Tile *)(p_tile->ext);

	// Destroy events of this tile
	Ctrl_GenericEvent_Release(p_tile_data->last_host_read_event);
	Ctrl_GenericEvent_Release(p_tile_data->last_host_write_event);

	// Free host image of the tile, equivalent to hit_tileFree(*p_tile);
	if (p_tile->memStatus == HIT_MS_OWNER) {
		// TODO @sergioalo move stuff to apropiate backend
		switch (p_tile_data->pinned) {
			case CTRL_TYPE_CPU:
				// global hwloc topology is the same that cpu ctrls use
				hwloc_free(topo, p_tile->memPtr, (size_t)p_tile->acumCard * p_tile->baseExtent);
				break;

			#ifdef _CTRL_ARCH_CUDA_
			case CTRL_TYPE_CUDA:
				CUDA_OP(cudaFreeHost(p_tile->memPtr));
				break;
			#endif //_CTRL_ARCH_CUDA_

			#ifdef _CTRL_ARCH_HIP_
			case CTRL_TYPE_HIP:
				HIP_OP(hipHostFree(p_tile->memPtr));
				break;
			#endif //_CTRL_ARCH_HIP_

			#if defined(_CTRL_ARCH_OPENCL_GPU_) || defined(_CTRL_ARCH_FPGA_)
			case CTRL_TYPE_OPENCL_GPU:
				OPENCL_ASSERT_OP(clEnqueueUnmapMemObject((cl_command_queue)p_tile_data->p_pin_queue, (cl_mem)p_tile_data->p_pinned_data, p_tile->memPtr, 0, NULL, NULL));
				OPENCL_ASSERT_OP(clFlush((cl_command_queue)p_tile_data->p_pin_queue));
				OPENCL_ASSERT_OP(clFinish((cl_command_queue)p_tile_data->p_pin_queue));
				OPENCL_ASSERT_OP(clReleaseMemObject((cl_mem)p_tile_data->p_pinned_data));
				OPENCL_ASSERT_OP(clReleaseCommandQueue((cl_command_queue)p_tile_data->p_pin_queue));
				break;
			#endif //_CTRL_ARCH_OPENCL_GPU_ || _CTRL_ARCH_FPGA_
			default:
				free(p_tile->memPtr);
				break;
		}
		p_tile->memPtr    = NULL;
		p_tile->data      = NULL;
		p_tile->memStatus = HIT_MS_NOMEM;
	}

	// Clear tile fields
	p_tile->ext = NULL;

	// Free tile
	free(p_tile_data->p_impls);
	free(p_tile_data);
}

///@endcond
