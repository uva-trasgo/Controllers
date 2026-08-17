///@cond INTERNAL
/**
 * @file Ctrl_OpenCL_Gpu.c
 * @brief Source code for OpenCL GPU backend.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "Architectures/OpenCL/Ctrl_OpenCL_Gpu.h"
#include "Core/Ctrl_Core.h"

/**
 * Head of the kernel params linked list.
 */
Ctrl_OpenCLGpu_KernelParams OpenCLGpu_initial_kp = CTRL_OPENCLGPU_KERNELPARAMS_NULL;

/**
 * Type id for the next OpenCLGPU ctrl.
 */
int next_oclgpu_id = 0;

/*************************************************************
 ******** Prototypes of private functions ********************
 *************************************************************/

/**
 * Initializate a \e Ctrl_OpenCL_Tile.
 *
 * @param p_ctrl Pointer to the ctrl attached to the tile and to be updated of the initialization.
 * @param p_task Pointer to the task containing the HitTile that contains the \e Ctrl_OpenCLGpu_Tile to be
 * initialized.
 */
void Ctrl_OpenCLGpu_InitTile(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task);

/**
 * Waits for all the work from \p p_ctrl related to \p p_tile_data .
 *
 * @param p_ctrl Pointer to the ctrl attached to the tile.
 * @param p_tile_data Pointer to the ctrl tile.
 *
 * @see Ctrl_OpenCLGpu_EvalTaskWaitTile, Ctrl_OpenCLGpu_EvalTaskGlobalSync
 */
void Ctrl_OpenCLGpu_WaitTileInner(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Tile *p_tile_data);

/**
 * Perform memory transfer from host to device.
 *
 * @param p_ctrl Pointer to the ctrl attached to the tile to be moved.
 * @param p_tile Pointer to the task containing the tile to be moved.
 *
 * @see Ctrl_OpenCLGpu_EvalTaskMoveTo
 */
void Ctrl_OpenCLGpu_EvalTaskMoveToInner(Ctrl_OpenCLGpu *p_ctrl, HitTile *p_tile);

/**
 * Perform memory transfer from host to device.
 *
 * @param p_task Task containing the tile to be moved.
 * @param p_ctrl Ctrl responsible for the task.
 *
 * @see Ctrl_OpenCLGpu_EvalTaskMoveTo
 */
void Ctrl_OpenCLGpu_ExecTaskMoveTo(Ctrl_Task *p_task, Ctrl_OpenCLGpu *p_ctrl);

/**
 * Perform memory transfer from device to host.
 *
 * @param p_ctrl Pointer to the ctrl attached to the tile to be moved.
 * @param p_tile Pointer to the task containing the tile to be moved.
 *
 * @see Ctrl_OpenCLGpu_EvalTaskMoveFrom
 */
void Ctrl_OpenCLGpu_EvalTaskMoveFromInner(Ctrl_OpenCLGpu *p_ctrl, HitTile *p_tile);

/**
 * Perform memory transfer from device to host.
 *
 * @param p_task Task containing the tile to be moved.
 * @param p_ctrl Ctrl responsible for the task.
 *
 * @see Ctrl_OpenCLGpu_EvalTaskMoveFrom
 */
void Ctrl_OpenCLGpu_ExecTaskMoveFrom(Ctrl_Task *p_task, Ctrl_OpenCLGpu *p_ctrl);

/**
 * Get \p qid th driver queue used by \p p_ctrl
 * Order of qid is htd, dth, kernels.
 *
 * @param p_ctrl Ctrl to get the queue from.
 * @param qid Pointer to the driver queues used by this ctrl
 * @return Queue \p qid of ctrl \p p_ctrl
 *
 * @pre \p qid must be between 0 and the amount of queues of \p p_ctrl
 * @see Ctrl_OpenCLGpu_GetNumQueues
 */
cl_command_queue Ctrl_OpenCLGpu_GetCmdQueueById(Ctrl_OpenCLGpu *p_ctrl, int qid);

/*************************************************************
 ******** Prototypes of tasks' evaluation functions **********
 *************************************************************/

/**
 * Destroy a OpenCLGpu ctrl.
 *
 * @param p_ctrl Ctrl to be destroyed.
 * @param p_task Task with info for cleanup
 */
void Ctrl_OpenCLGpu_Destroy(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of kernel launch.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_OpenCLGpu_EvalTask, Ctrl_Launch
 */
void Ctrl_OpenCLGpu_EvalTaskKernelLaunch(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of global sync. Waits for all work related to any tile attached to this ctrl.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_OpenCLGpu_EvalTask, Ctrl_GlobalSync
 */
void Ctrl_OpenCLGpu_EvalTaskGlobalSync(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of allocation of tiles.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_OpenCLGpu_EvalTask, Ctrl_Alloc
 */
void Ctrl_OpenCLGpu_EvalTaskAllocTile(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of subselecting tiles.
 *
 * @param p_ctrl: Ctrl in charge of task.
 * @param p_task: task to be evaluated.
 *
 * @see Ctrl_OpenCLGpu_EvalTask, Ctrl_Select
 */
void Ctrl_OpenCLGpu_EvalTaskSelectTile(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of freeing of tiles.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_OpenCLGpu_EvalTask, Ctrl_Free
 */
void Ctrl_OpenCLGpu_EvalTaskFreeTile(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of comunication of tile from host to device.
 *
 * This function checks state of tile and, if comunication is necesary calls to \e Ctrl_OpenCLGpu_EvalTaskMoveToInner.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_OpenCLGpu_EvalTask, Ctrl_OpenCLGpu_EvalTaskMoveToInner, Ctrl_MoveTo
 */
void Ctrl_OpenCLGpu_EvalTaskMoveTo(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of comunication of tile from host to device.
 *
 * This function checks state of tile and, if comunication is necesary calls to \e Ctrl_OpenCLGpu_EvalTaskMoveToInner.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_OpenCLGpu_EvalTask, Ctrl_OpenCLGpu_EvalTaskMoveToInner, Ctrl_MoveTo
 */
void Ctrl_OpenCLGpu_EvalTaskMoveFrom(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of wait. Waits for all the work related to the tile specified in \p p_task .
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_OpenCLGpu_EvalTask, Ctrl_WaitTile
 */
void Ctrl_OpenCLGpu_EvalTaskWaitTile(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of change of dependance mode.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated. .
 */
void Ctrl_OpenCLGpu_EvalTaskSetDependanceMode(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task);

/********************************************
 ***** OpenCL GPU Controller functions ******
 ********************************************/

void Ctrl_OpenCLGpu_Create(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Policy policy, Ctrl_Config_Dev dev) {
	cl_int err;

	p_ctrl->policy    = policy;
	p_ctrl->type_id   = next_oclgpu_id++;
	p_ctrl->alignment = atoi(Ctrl_Config_GetVal(dev, "align", "0"));

	int platform = atoi(Ctrl_Config_GetVal(dev, "platform", NULL));
	int device   = atoi(Ctrl_Config_GetVal(dev, "dev", NULL));

	p_ctrl->n_kernel_streams = atoi(Ctrl_Config_GetVal(dev, "kstreams", "1"));
	if (p_ctrl->n_kernel_streams <= 0) {
		p_ctrl->n_kernel_streams = 1;
		fprintf(stderr, "[Ctrl_OpenCL_Gpu] Warning: Tried to create OpenCL_Gpu Ctrl with less than one queue; defaulting to 1.\n");
		fflush(stderr);
	}
	p_ctrl->p_kernel_driver_streams = (cl_command_queue *)malloc(p_ctrl->n_kernel_streams * sizeof(cl_command_queue));
	p_ctrl->pp_kernel_host_streams  = (Ctrl_TaskQueue **)malloc(p_ctrl->n_kernel_streams * sizeof(Ctrl_TaskQueue *));
	p_ctrl->dependance_mode         = CTRL_MODE_IMPLICIT;

	// get OpenCL platform id from platform index
	cl_platform_id *p_platform_ids = (cl_platform_id *)malloc((platform + 1) * sizeof(cl_platform_id));
	OPENCL_ASSERT_OP(clGetPlatformIDs(platform + 1, p_platform_ids, NULL));
	p_ctrl->platform_id = p_platform_ids[platform];
	free(p_platform_ids);

	size_t platform_name_size;
	OPENCL_ASSERT_OP(clGetPlatformInfo(p_ctrl->platform_id, CL_PLATFORM_NAME, 0, NULL, &platform_name_size));
	char *platform_name = (char *)malloc(sizeof(char) * platform_name_size);
	OPENCL_ASSERT_OP(clGetPlatformInfo(p_ctrl->platform_id, CL_PLATFORM_NAME, platform_name_size, platform_name, NULL));

	// get OpenCL device id from device index
	cl_device_id *p_device_ids = (cl_device_id *)malloc((device + 1) * sizeof(cl_device_id));
	OPENCL_ASSERT_OP(clGetDeviceIDs(p_ctrl->platform_id, CL_DEVICE_TYPE_GPU, device + 1, p_device_ids, NULL));
	p_ctrl->device_id = p_device_ids[device];
	free(p_device_ids);

	size_t device_name_size;
	OPENCL_ASSERT_OP(clGetDeviceInfo(p_ctrl->device_id, CL_DEVICE_NAME, 0, NULL, &device_name_size));
	char device_name[device_name_size];
	OPENCL_ASSERT_OP(clGetDeviceInfo(p_ctrl->device_id, CL_DEVICE_NAME, device_name_size, device_name, NULL));

	if (strstr(platform_name, "AMD")) { // some amd devices have performance issues when using pinned
		if (strstr(device_name, "gfx900")) {
			p_ctrl->default_alloc_mode = CTRL_MEM_NOPINNED;
		} else {
			p_ctrl->default_alloc_mode = CTRL_MEM_PINNED;
		}
	} else if (strstr(platform_name, "NVIDIA")) { // nvidia platforms prefer pinned memory
		p_ctrl->default_alloc_mode = CTRL_MEM_PINNED;
	} else { // on other unknown platforms default to using "pinned mem"
		p_ctrl->default_alloc_mode = CTRL_MEM_PINNED;
	}
	free(platform_name);

	// Create OpenCL context
	cl_context_properties context_properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)p_ctrl->platform_id, 0};
	p_ctrl->context                            = clCreateContext(context_properties, 1, &(p_ctrl->device_id), NULL, NULL, &err);
	OPENCL_ASSERT_ERROR(err);

	p_ctrl->queue_properties = 0;
	// Enable op timing
	p_ctrl->queue_properties |= CL_QUEUE_PROFILING_ENABLE;

	p_ctrl->p_tile_list_head = NULL;
	p_ctrl->p_tile_list_tail = NULL;

	// Create OpenCL queues for kernel execution
	p_ctrl->htd_driver_stream = clCreateCommandQueue(p_ctrl->context, p_ctrl->device_id, p_ctrl->queue_properties, &err);
	OPENCL_ASSERT_ERROR(err);
	p_ctrl->dth_driver_stream = clCreateCommandQueue(p_ctrl->context, p_ctrl->device_id, p_ctrl->queue_properties, &err);
	OPENCL_ASSERT_ERROR(err);
	p_ctrl->pin_queue = clCreateCommandQueue(p_ctrl->context, p_ctrl->device_id, p_ctrl->queue_properties, &err);
	OPENCL_ASSERT_ERROR(err);
	p_ctrl->p_htd_host_stream = Ctrl_TaskQueue_Create();
	p_ctrl->p_dth_host_stream = Ctrl_TaskQueue_Create();
	for (int i = 0; i < p_ctrl->n_kernel_streams; i++) {
		p_ctrl->p_kernel_driver_streams[i] = clCreateCommandQueue(p_ctrl->context, p_ctrl->device_id, p_ctrl->queue_properties, &err);
		OPENCL_ASSERT_ERROR(err);
		p_ctrl->pp_kernel_host_streams[i] = Ctrl_TaskQueue_Create();
	}

	// create events
	p_ctrl->host_seq_event = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_CPU, p_ctrl->global_id);
	p_ctrl->dev_seq_event  = CTRL_GENERIC_EVENT_NULL;

	for (Ctrl_OpenCLGpu_KernelParams *p_curr_kp = OpenCLGpu_initial_kp.p_next; p_curr_kp != NULL; p_curr_kp = p_curr_kp->p_next) {
		size_t kernel_raw_size = strlen(p_curr_kp->p_kernel_raw);

		p_curr_kp->p_program[p_ctrl->type_id] = clCreateProgramWithSource(p_ctrl->context, 1, (const char **)(&p_curr_kp->p_kernel_raw), &kernel_raw_size, &err);
		OPENCL_ASSERT_ERROR(err);
		err = clBuildProgram(p_curr_kp->p_program[p_ctrl->type_id], 1, &p_ctrl->device_id, NULL, NULL, NULL);
		if (err == CL_BUILD_PROGRAM_FAILURE) {
			fprintf(stderr, "opencl kernel build faliure\n");
			fprintf(stderr, "\n\n%s\n\n", p_curr_kp->p_kernel_raw);
			fflush(stderr);
			size_t log_size;
			clGetProgramBuildInfo(p_curr_kp->p_program[p_ctrl->type_id], p_ctrl->device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
			char *log = (char *)malloc(log_size);
			clGetProgramBuildInfo(p_curr_kp->p_program[p_ctrl->type_id], p_ctrl->device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
			fprintf(stderr, "%s\n", log);
			fflush(stderr);
			free(log);
		}
		OPENCL_ASSERT_ERROR(err);
		p_curr_kp->p_kernel[p_ctrl->type_id] = clCreateKernel(p_curr_kp->p_program[p_ctrl->type_id], (const char *)p_curr_kp->p_kernel_name, &err);
		OPENCL_ASSERT_ERROR(err);
	}

	// create a dummy image to pass to kernels when textures are not used
	cl_image_format image_fmt         = {0};
	image_fmt.image_channel_order     = CL_R;             // single channel
	image_fmt.image_channel_data_type = CL_UNSIGNED_INT8; // smallest datatype

	cl_image_desc image_desc    = {0};
	image_desc.image_type       = CL_MEM_OBJECT_IMAGE2D;
	image_desc.image_width      = 1;
	image_desc.image_height     = 1;
	image_desc.image_array_size = 1;

	p_ctrl->default_2dimg = clCreateImage(p_ctrl->context, CL_MEM_READ_WRITE, &image_fmt, &image_desc, NULL, &err);
	OPENCL_ASSERT_ERROR(err);

	#ifdef _CTRL_OPENCL_GPU_PROFILING_
	p_ctrl->platform = platform;
	p_ctrl->device   = device;

	p_ctrl->profiling_info_start = CL_PROFILING_COMMAND_START;
	p_ctrl->profiling_info_end   = CL_PROFILING_COMMAND_END;

	p_ctrl->profiling_read_events   = (cl_event *)malloc(_OPENCL_GPU_PROFILING_N_READ_TASKS_ * sizeof(cl_event));
	p_ctrl->profiling_write_events  = (cl_event *)malloc(_OPENCL_GPU_PROFILING_N_WRITE_TASKS_ * sizeof(cl_event));
	p_ctrl->profiling_kernel_events = (cl_event *)malloc(_OPENCL_GPU_PROFILING_N_KERNEL_TASKS_ * sizeof(cl_event));

	p_ctrl->i_read_task   = 0;
	p_ctrl->i_write_task  = 0;
	p_ctrl->i_kernel_task = 0;

	p_ctrl->profiling_total      = 0;
	p_ctrl->profiling_sum        = 0;
	p_ctrl->profiling_offloading = 0;
	p_ctrl->profiling_read       = 0;
	p_ctrl->profiling_write      = 0;
	p_ctrl->profiling_kernel     = 0;

	p_ctrl->profiling_start = 0;
	p_ctrl->profiling_end   = 0;

	p_ctrl->has_first_profiling_event = false;

	p_ctrl->last_profiling_event = p_ctrl->default_event;
	OPENCL_ASSERT_OP(clRetainEvent(p_ctrl->last_profiling_event));

	#ifdef _CTRL_OPENCL_GPU_PROFILING_VERBOSE_
	p_ctrl->profiling_visual_events = (visual_event *)malloc(
		(_OPENCL_GPU_PROFILING_N_READ_TASKS_ +
		 _OPENCL_GPU_PROFILING_N_WRITE_TASKS_ +
		 _OPENCL_GPU_PROFILING_N_KERNEL_TASKS_) *
		sizeof(visual_event));
	p_ctrl->i_visual_task = 0;
	#endif
	#endif // _CTRL_OPENCL_GPU_PROFILING_
}

void Ctrl_OpenCLGpu_EvalTask(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task) {
	switch (p_task->task_type) {
		case CTRL_TASK_TYPE_KERNEL:
			Ctrl_OpenCLGpu_EvalTaskKernelLaunch(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_GLOBALSYNC:
			Ctrl_OpenCLGpu_EvalTaskGlobalSync(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_ALLOCTILE:
			Ctrl_OpenCLGpu_EvalTaskAllocTile(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_SELECTTILE:
			Ctrl_OpenCLGpu_EvalTaskSelectTile(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_FREETILE:
			Ctrl_OpenCLGpu_EvalTaskFreeTile(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_MOVETO:
			Ctrl_OpenCLGpu_EvalTaskMoveTo(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_MOVEFROM:
			Ctrl_OpenCLGpu_EvalTaskMoveFrom(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_WAITTILE:
			Ctrl_OpenCLGpu_EvalTaskWaitTile(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_DESTROYCTRL:
			Ctrl_OpenCLGpu_Destroy(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_SETDEPENDANCEMODE:
			Ctrl_OpenCLGpu_EvalTaskSetDependanceMode(p_ctrl, p_task);
			break;
		default:
			fprintf(stderr, "[Ctrl_OpenCL_Gpu] Unsupported task type: %d.\n", p_task->task_type);
			exit(EXIT_FAILURE);
	}
}

void Ctrl_OpenCLGpu_AllocKernel(int n_oclgpu_ctrls) {
	for (Ctrl_OpenCLGpu_KernelParams *p_curr_kp = OpenCLGpu_initial_kp.p_next; p_curr_kp != NULL; p_curr_kp = p_curr_kp->p_next) {
		p_curr_kp->p_program = (cl_program *)malloc(n_oclgpu_ctrls * sizeof(cl_program));
		p_curr_kp->p_kernel  = (cl_kernel *)malloc(n_oclgpu_ctrls * sizeof(cl_kernel));
	}
}

void Ctrl_OpenCLGpu_ExecTask(Ctrl_Task *p_task, Ctrl_OpenCLGpu *p_ctrl) {
	cl_command_queue cmd_queue   = Ctrl_OpenCLGpu_GetCmdQueueById(p_ctrl, p_task->stream);
	p_task->request.opencl.queue = &cmd_queue;
	switch (p_task->task_type) {
		case CTRL_TASK_TYPE_KERNEL:
			p_task->pfn_kernel_wrapper(p_task->request, p_task->device_id, CTRL_TYPE_OPENCL_GPU, p_task->threads, p_task->blocksize, p_task->p_arguments);
			Ctrl_GenericEvent_Release(p_task->event);
			break;
		case CTRL_TASK_TYPE_MOVETO:
			Ctrl_OpenCLGpu_ExecTaskMoveTo(p_task, p_ctrl);
			Ctrl_GenericEvent_Release(p_task->event);
			break;
		case CTRL_TASK_TYPE_MOVEFROM:
			Ctrl_OpenCLGpu_ExecTaskMoveFrom(p_task, p_ctrl);
			Ctrl_GenericEvent_Release(p_task->event);
			break;
		case CTRL_TASK_TYPE_WAITEVENT:
			OPENCL_ASSERT_OP(clEnqueueBarrierWithWaitList(cmd_queue, 1, p_task->event.event.p_event_cl, NULL));
			break;
		default:
			fprintf(stderr, "[Ctrl_OpenCLGpu] ExecTask: task type %d should not get here\n", p_task->task_type);
			exit(EXIT_FAILURE);
	}
}

int Ctrl_OpenCLGpu_GetNumQueues(Ctrl_OpenCLGpu *p_ctrl) {
	return p_ctrl->n_kernel_streams + 2;
}

Ctrl_TaskQueue **Ctrl_OpenCLGpu_GetHostQueues(Ctrl_OpenCLGpu *p_ctrl, Ctrl_TaskQueue **pp_queues) {
	pp_queues[0] = p_ctrl->p_htd_host_stream;
	pp_queues[1] = p_ctrl->p_dth_host_stream;

	for (int i = 0; i < p_ctrl->n_kernel_streams; i++) {
		pp_queues[i + 2] = p_ctrl->pp_kernel_host_streams[i];
	}

	return &pp_queues[Ctrl_OpenCLGpu_GetNumQueues(p_ctrl)];
}

cl_command_queue Ctrl_OpenCLGpu_GetCmdQueueById(Ctrl_OpenCLGpu *p_ctrl, int qid) {
	if (qid < 0 || qid > Ctrl_OpenCLGpu_GetNumQueues(p_ctrl)) {
		fprintf(stderr, "[Ctrl_OpenCLGpu_GetCmdQueueById] Invalid qid %d\n", qid);
		exit(EXIT_FAILURE);
	}

	switch (qid) {
		case 0:
			return p_ctrl->htd_driver_stream;
		case 1:
			return p_ctrl->dth_driver_stream;
		default:
			return p_ctrl->p_kernel_driver_streams[qid - 2];
	}
}

void Ctrl_OpenCLGpu_GetInfo(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Info *p_info) {
	p_info->type = "OpenCL_GPU";

	size_t platform_name_size;
	OPENCL_ASSERT_OP(clGetPlatformInfo(p_ctrl->platform_id, CL_PLATFORM_NAME, 0, NULL, &platform_name_size));
	char platform_name[platform_name_size];
	OPENCL_ASSERT_OP(clGetPlatformInfo(p_ctrl->platform_id, CL_PLATFORM_NAME, platform_name_size, platform_name, NULL));

	size_t device_name_size;
	OPENCL_ASSERT_OP(clGetDeviceInfo(p_ctrl->device_id, CL_DEVICE_NAME, 0, NULL, &device_name_size));
	char device_name[device_name_size];
	OPENCL_ASSERT_OP(clGetDeviceInfo(p_ctrl->device_id, CL_DEVICE_NAME, device_name_size, device_name, NULL));

	strncpy(p_info->device_name, device_name, CTRL_MAX_DEV_NAME - 1);
	strncpy(p_info->platform_name, platform_name, CTRL_MAX_DEV_NAME - 1);
	p_info->device_name[255]   = '\0';
	p_info->platform_name[255] = '\0';
	p_info->n_kernel_queues    = p_ctrl->n_kernel_streams;
}

double Ctrl_OpenCLGpu_TimeLastOp(Ctrl_OpenCLGpu *p_ctrl, HitTile *p_tile) {
	Ctrl_Tile        *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl   *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_OpenCL_Tile *p_tile_data_ocl  = p_tile_data_impl->tile.p_opencl;

	cl_ulong time_start;
	cl_ulong time_end;

	clGetEventProfilingInfo(*p_tile_data_ocl->last_op.event.p_event_cl, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
	clGetEventProfilingInfo(*p_tile_data_ocl->last_op.event.p_event_cl, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

	return (time_end - time_start) / 1.0e9;
}

void Ctrl_OpenCLGpu_CreateTex(Ctrl_OpenCLGpu *p_ctrl, HitTile *p_tile, Ctrl_TexDesc tex_desc) {
	cl_int err;

	Ctrl_Tile        *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl   *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_OpenCL_Tile *p_tile_data_ocl  = p_tile_data_impl->tile.p_opencl;

	if (hit_tileDims(*p_tile) != 2) {
		fprintf(stderr, "[Ctrl_OpenCLGPU_CreateTex] Error: textures are currently only supported for 2D matrixes.");
		exit(EXIT_FAILURE);
	}

	if (p_tile_data_impl->type == CTRL_TYPE_NULL) {
		fprintf(stderr, "[Ctrl_OpenCLGPU_CreateTex] Error: tile not asociated with ctrl.");
		exit(EXIT_FAILURE);
	}

	if (p_tile_data_impl->device_memowner == HIT_MS_NOMEM) {
		fprintf(stderr, "[Ctrl_OpenCLGPU_CreateTex] Error: Device memory not allocated for this tile. You must allocate device memory before creating textures.\n");
		exit(EXIT_FAILURE);
	}

	// TODO @sergioalo check if pitch used to alloc the tile is valid for texture
	cl_uint device_pitch;
	OPENCL_ASSERT_OP(clGetDeviceInfo(p_ctrl->device_id, CL_DEVICE_IMAGE_PITCH_ALIGNMENT, sizeof(cl_uint), &device_pitch, NULL));
	if (device_pitch == 0) {
		fprintf(stderr, "[Ctrl_OpenCLGPU_CreateTex] Error: Unsupported device. Current Controller OpenCL texture implementation only supports creating textures from existing buffers. This is a OpenCL 2.x feature.\n");
		exit(EXIT_FAILURE);
	}

	// TODO @sergioalo img format stuff should not be hardcoded
	// TODO @sergioalo check how to destroy textures and samplers

	// Define the image format
	cl_image_format image_fmt;
	image_fmt.image_channel_order     = CL_R; // Single-channel (red)
	image_fmt.image_channel_data_type = CL_FLOAT;

	// Create the image from the existing buffer
	cl_image_desc image_desc;
	image_desc.image_type        = CL_MEM_OBJECT_IMAGE2D;
	image_desc.image_width       = tex_desc.width == 0 ? hit_tileDimCard(*p_tile, 1) : tex_desc.width;
	image_desc.image_height      = tex_desc.height == 0 ? hit_tileDimCard(*p_tile, 0) : tex_desc.height;
	image_desc.image_array_size  = 1;
	image_desc.image_row_pitch   = p_tile_data_impl->origAcumCard[1] * p_tile->baseExtent;
	image_desc.image_slice_pitch = 0;
	image_desc.num_mip_levels    = 0;
	image_desc.num_samples       = 0;
	image_desc.buffer            = p_tile_data_ocl->device_data;

	p_tile_data_ocl->texture = clCreateImage(p_ctrl->context, CL_MEM_READ_WRITE, &image_fmt, &image_desc, NULL, &err);
	OPENCL_ASSERT_ERROR(err);

	// convert ctrl texture options to opencl sampler settings
	cl_addressing_mode addr_mode = 0;
	switch (tex_desc.addr_mode[0]) {
		case CTRL_TEX_ADDRMODE_BORDER:
			addr_mode = CL_ADDRESS_CLAMP;
			break;
		case CTRL_TEX_ADDRMODE_CLAMP:
			addr_mode = CL_ADDRESS_CLAMP_TO_EDGE;
			break;
		case CTRL_TEX_ADDRMODE_MIRROR:
			addr_mode = CL_ADDRESS_MIRRORED_REPEAT;
			break;
		case CTRL_TEX_ADDRMODE_WRAP:
			addr_mode = CL_ADDRESS_REPEAT;
			break;
	}

	cl_filter_mode filter_mode = 0;
	switch (tex_desc.filter_mode) {
		case CTRL_TEX_FILTERMODE_POINT:
			filter_mode = CL_FILTER_NEAREST;
			break;
		case CTRL_TEX_FILTERMODE_LINEAR:
			filter_mode = CL_FILTER_LINEAR;
			break;
	}

	p_tile_data_ocl->sampler = clCreateSampler(p_ctrl->context, tex_desc.normalized_coords, addr_mode, filter_mode, &err);
	OPENCL_ASSERT_ERROR(err);
}

/*********************************
 ******* Private functions *******
 *********************************/

void Ctrl_OpenCLGpu_InitTile(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task) {
	cl_int err;

	Ctrl_Tile *p_tile_data = (Ctrl_Tile *)(p_task->p_tile->ext);
	p_tile_data->valid_impls++;

	Ctrl_Tile_Impl   *p_tile_data_impl     = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_OpenCL_Tile *p_tile_data_impl_ocl = (Ctrl_OpenCL_Tile *)malloc(sizeof(Ctrl_OpenCL_Tile));
	p_tile_data_impl->type                 = CTRL_TYPE_OPENCL_GPU;
	p_tile_data_impl->tile.p_opencl        = p_tile_data_impl_ocl;
	p_tile_data_impl->device_status        = CTRL_TILE_INVALID;
	p_tile_data_impl->device_memowner      = HIT_MS_NOMEM;

	p_tile_data_impl_ocl->p_ctrl  = p_ctrl;
	p_tile_data_impl_ocl->texture = NULL;
	// sampler and img can never be null so we initialize it to default stuff
	p_tile_data_impl_ocl->texture = p_ctrl->default_2dimg;
	p_tile_data_impl_ocl->sampler = clCreateSampler(p_ctrl->context, CL_FALSE, CL_ADDRESS_NONE, CL_FILTER_NEAREST, &err);
	OPENCL_ASSERT_ERROR(err);

	// create node for the the new tile
	Ctrl_Tile_List *p_list_node       = (Ctrl_Tile_List *)malloc(sizeof(Ctrl_Tile_List));
	p_list_node->p_prev               = NULL;
	p_list_node->p_next               = NULL;
	p_list_node->p_tile_ext           = p_tile_data;
	p_tile_data_impl_ocl->p_tile_elem = p_list_node;

	// insert node into the linked list of tiles
	if (p_ctrl->p_tile_list_tail != NULL) {
		p_ctrl->p_tile_list_tail->p_next = p_list_node;
		p_list_node->p_prev              = p_ctrl->p_tile_list_tail;
		p_ctrl->p_tile_list_tail         = p_list_node;
	} else {
		p_ctrl->p_tile_list_head = p_ctrl->p_tile_list_tail = p_list_node;
	}

	// Create events for this tile
	p_tile_data_impl_ocl->host_last_kernel_read_event  = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_CPU, p_ctrl->global_id);
	p_tile_data_impl_ocl->host_last_kernel_write_event = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_CPU, p_ctrl->global_id);
	p_tile_data_impl_ocl->host_last_dth_event          = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_CPU, p_ctrl->global_id);
	p_tile_data_impl_ocl->host_last_htd_event          = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_CPU, p_ctrl->global_id);
	p_tile_data_impl_ocl->dev_last_kernel_read_event   = CTRL_GENERIC_EVENT_NULL;
	p_tile_data_impl_ocl->dev_last_kernel_write_event  = CTRL_GENERIC_EVENT_NULL;
	p_tile_data_impl_ocl->dev_last_dth_event           = CTRL_GENERIC_EVENT_NULL;
	p_tile_data_impl_ocl->dev_last_htd_event           = CTRL_GENERIC_EVENT_NULL;
	p_tile_data_impl_ocl->last_op                      = CTRL_GENERIC_EVENT_NULL;

	p_tile_data_impl_ocl->streamid_last_kr = 0;
	p_tile_data_impl_ocl->streamid_last_kw = 0;
}

void Ctrl_OpenCLGpu_WaitTileInner(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Tile *p_tile_data) {
	Ctrl_OpenCL_Tile *p_tile_data_ocl = p_tile_data->p_impls[p_ctrl->global_id].tile.p_opencl;

	// Wait for all work related to this tile to finish
	Ctrl_GenericEvent_Wait(p_tile_data->last_host_read_event);
	Ctrl_GenericEvent_Wait(p_tile_data->last_host_write_event);
	Ctrl_GenericEvent_Wait(p_tile_data_ocl->host_last_kernel_read_event);
	Ctrl_GenericEvent_Wait(p_tile_data_ocl->host_last_kernel_write_event);
	Ctrl_GenericEvent_Wait(p_tile_data_ocl->host_last_dth_event);
	Ctrl_GenericEvent_Wait(p_tile_data_ocl->host_last_htd_event);
	Ctrl_GenericEvent_Wait(p_tile_data_ocl->dev_last_kernel_read_event);
	Ctrl_GenericEvent_Wait(p_tile_data_ocl->dev_last_kernel_write_event);
	Ctrl_GenericEvent_Wait(p_tile_data_ocl->dev_last_dth_event);
	Ctrl_GenericEvent_Wait(p_tile_data_ocl->dev_last_htd_event);

	Ctrl_GenericEvent_Release(p_tile_data_ocl->dev_last_kernel_read_event);
	Ctrl_GenericEvent_Release(p_tile_data_ocl->dev_last_kernel_write_event);
	Ctrl_GenericEvent_Release(p_tile_data_ocl->dev_last_dth_event);
	Ctrl_GenericEvent_Release(p_tile_data_ocl->dev_last_htd_event);

	p_tile_data_ocl->dev_last_kernel_read_event  = CTRL_GENERIC_EVENT_NULL;
	p_tile_data_ocl->dev_last_kernel_write_event = CTRL_GENERIC_EVENT_NULL;
	p_tile_data_ocl->dev_last_dth_event          = CTRL_GENERIC_EVENT_NULL;
	p_tile_data_ocl->dev_last_htd_event          = CTRL_GENERIC_EVENT_NULL;
}

/* Macro to define MoveTo and MoveFrom logic */
#define OpenCL_Move(type)                                                                              \
	size_t zero_offset[3] = {0};       /* xyz */                                                       \
	size_t size[3]        = {1, 1, 1}; /* xyz */                                                       \
	size_t dev_offset[3]  = {0};       /* xyz */                                                       \
	dev_offset[0]         = p_tile_data_impl->offset * p_tile->baseExtent;                             \
	int transfer_dims     = ((hit_tileDims(*p_tile) < 3) ? hit_tileDims(*p_tile) : 3);                 \
	for (int i = 0; i < transfer_dims; i++) {                                                          \
		size[i] = p_tile->card[hit_tileDims(*p_tile) - 1 - i];                                         \
	}                                                                                                  \
	size[0] *= p_tile->baseExtent;                                                                     \
	switch (hit_tileDims(*p_tile)) {                                                                   \
		case 1:                                                                                        \
			OPENCL_ASSERT_OP(                                                                          \
				clEnqueue##type##Buffer(cmd_queue, p_tile_data_ocl->device_data,                       \
										CL_FALSE, dev_offset[0],                                       \
										(size_t)p_tile->acumCard * p_tile->baseExtent,                 \
										p_tile->data, 0, NULL,                                         \
										p_task->event.event.p_event_cl));                              \
			break;                                                                                     \
		case 2:                                                                                        \
			OPENCL_ASSERT_OP(                                                                          \
				clEnqueue##type##BufferRect(                                                           \
					cmd_queue,                                                                         \
					p_tile_data_ocl->device_data,                                                      \
					CL_FALSE, dev_offset, zero_offset, size,                                           \
					p_tile->baseExtent * p_tile_data_impl->origAcumCard[1], 0,                         \
					p_tile->baseExtent * p_tile->origAcumCard[1], 0,                                   \
					p_tile->data, 0, NULL,                                                             \
					p_task->event.event.p_event_cl));                                                  \
			break;                                                                                     \
		case 3:                                                                                        \
			OPENCL_ASSERT_OP(                                                                          \
				clEnqueue##type##BufferRect(                                                           \
					cmd_queue,                                                                         \
					p_tile_data_ocl->device_data,                                                      \
					CL_FALSE, dev_offset, zero_offset, size,                                           \
					p_tile->baseExtent * p_tile_data_impl->origAcumCard[2],                            \
					p_tile->baseExtent * p_tile_data_impl->origAcumCard[1],                            \
					p_tile->baseExtent * p_tile->origAcumCard[2],                                      \
					p_tile->baseExtent * p_tile->origAcumCard[1],                                      \
					p_tile->data, 0, NULL,                                                             \
					p_task->event.event.p_event_cl));                                                  \
			break;                                                                                     \
		case 4:                                                                                        \
			char *p_host_data = (char *)p_tile->data;                                                  \
			for (int i = 0; i < hit_tileDimCard(*p_tile, 0); i++) {                                    \
				OPENCL_ASSERT_OP(                                                                      \
					clEnqueue##type##BufferRect(                                                       \
						cmd_queue,                                                                     \
						p_tile_data_ocl->device_data,                                                  \
						CL_FALSE, dev_offset, zero_offset, size,                                       \
						p_tile->baseExtent * p_tile_data_impl->origAcumCard[3],                        \
						p_tile->baseExtent * p_tile_data_impl->origAcumCard[2],                        \
						p_tile->baseExtent * p_tile->origAcumCard[3],                                  \
						p_tile->baseExtent * p_tile->origAcumCard[2],                                  \
						(void *)p_host_data, 0, NULL,                                                  \
						p_task->event.event.p_event_cl));                                              \
				dev_offset[0] += p_tile_data_impl->origAcumCard[1] * p_tile->baseExtent;               \
				p_host_data += p_tile->origAcumCard[1] * p_tile->baseExtent;                           \
			}                                                                                          \
			break;                                                                                     \
		default:                                                                                       \
			fprintf(stderr, "[Ctrl_OpenCL_Gpu] Internal Error: "                                       \
							"Number of dimensions not supported for non-owner tile in mem move: %d\n", \
					hit_tileDims(*p_tile));                                                            \
			exit(EXIT_FAILURE);                                                                        \
	}

void Ctrl_OpenCLGpu_EvalTaskMoveToInner(Ctrl_OpenCLGpu *p_ctrl, HitTile *p_tile) {
	Ctrl_Tile        *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl   *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_OpenCL_Tile *p_tile_data_ocl  = p_tile_data_impl->tile.p_opencl;

	for (int i = 0; i < Ctrl_GetNCtrls(); i++) {
		if (i == p_ctrl->global_id) continue;

		Ctrl_MoveToWait(&p_tile_data->p_impls[i], p_ctrl->p_htd_host_stream);
	}

	Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->host_last_kernel_read_event, p_ctrl->p_htd_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->host_last_kernel_write_event, p_ctrl->p_htd_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data->last_host_write_event, p_ctrl->p_htd_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->dev_last_kernel_read_event, p_ctrl->p_htd_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->dev_last_kernel_write_event, p_ctrl->p_htd_host_stream);

	// Wait for previous task to finish if policy is sync
	Ctrl_SyncWait(p_ctrl->p_htd_host_stream);

	// Replace previous htd event
	Ctrl_GenericEvent_Release(p_tile_data_ocl->dev_last_htd_event);
	p_tile_data_ocl->dev_last_htd_event = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_OPENCL, p_ctrl->global_id);

	// Replace previous last op event
	Ctrl_GenericEvent_Release(p_tile_data_ocl->last_op);
	p_tile_data_ocl->last_op = p_tile_data_ocl->dev_last_htd_event;
	Ctrl_GenericEvent_Retain(p_tile_data_ocl->last_op);

	Ctrl_Task task = CTRL_TASK_NULL;
	task.task_type = CTRL_TASK_TYPE_MOVETO;
	task.event     = p_tile_data_ocl->dev_last_htd_event;
	task.tile      = *p_tile;
	Ctrl_GenericEvent_Retain(task.event);
	Ctrl_TaskQueue_Push(p_ctrl->p_htd_host_stream, task);
	Ctrl_CpuEvent_Record(&p_tile_data_ocl->host_last_htd_event.event.event_cpu, p_ctrl->p_htd_host_stream);

	if (p_ctrl->policy == CTRL_POLICY_SYNC) {
		Ctrl_CpuEvent_Record(&p_ctrl->host_seq_event.event.event_cpu, p_ctrl->p_htd_host_stream);
		Ctrl_GenericEvent_Release(p_ctrl->dev_seq_event);
		p_ctrl->dev_seq_event = p_tile_data_ocl->dev_last_htd_event;
		Ctrl_GenericEvent_Retain(p_ctrl->dev_seq_event);
	}

	// Update state of the tile
	p_tile_data->host_status        = CTRL_TILE_VALID;
	p_tile_data_impl->device_status = CTRL_TILE_VALID;
}

void Ctrl_OpenCLGpu_MoveToWait(Ctrl_OpenCL_Tile *p_tile_data, Ctrl_TaskQueue *p_queue) {
	Ctrl_GenericEvent_StreamWait(p_tile_data->host_last_dth_event, p_queue);
	Ctrl_GenericEvent_StreamWait(p_tile_data->dev_last_dth_event, p_queue);
}

void Ctrl_OpenCLGpu_ExecTaskMoveTo(Ctrl_Task *p_task, Ctrl_OpenCLGpu *p_ctrl) {
	HitTile          *p_tile           = &p_task->tile;
	Ctrl_Tile        *p_tile_data      = (Ctrl_Tile *)p_tile->ext;
	Ctrl_Tile_Impl   *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_OpenCL_Tile *p_tile_data_ocl  = p_tile_data_impl->tile.p_opencl;
	cl_command_queue  cmd_queue        = p_ctrl->htd_driver_stream;

	// Enqueue the transfer operation
	OpenCL_Move(Write);

	OPENCL_ASSERT_OP(clFlush(cmd_queue));
}

void Ctrl_OpenCLGpu_EvalTaskMoveFromInner(Ctrl_OpenCLGpu *p_ctrl, HitTile *p_tile) {
	Ctrl_Tile        *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl   *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_OpenCL_Tile *p_tile_data_ocl  = p_tile_data_impl->tile.p_opencl;

	for (int i = 0; i < Ctrl_GetNCtrls(); i++) {
		if (i == p_ctrl->global_id) continue;

		Ctrl_MoveFromWait(&p_tile_data->p_impls[i], p_ctrl->p_dth_host_stream);
	}

	// Wait for appropiate events
	Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->host_last_kernel_write_event, p_ctrl->p_dth_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data->last_host_read_event, p_ctrl->p_dth_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data->last_host_write_event, p_ctrl->p_dth_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->dev_last_kernel_write_event, p_ctrl->p_dth_host_stream);

	// wait for previous task to finish if policy is sync
	Ctrl_SyncWait(p_ctrl->p_dth_host_stream);

	// Replace previous dth event
	Ctrl_GenericEvent_Release(p_tile_data_ocl->dev_last_dth_event);
	p_tile_data_ocl->dev_last_dth_event = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_OPENCL, p_ctrl->global_id);

	// Replace previous last op event
	Ctrl_GenericEvent_Release(p_tile_data_ocl->last_op);
	p_tile_data_ocl->last_op = p_tile_data_ocl->dev_last_dth_event;
	Ctrl_GenericEvent_Retain(p_tile_data_ocl->last_op);

	Ctrl_Task task = CTRL_TASK_NULL;
	task.task_type = CTRL_TASK_TYPE_MOVEFROM;
	task.event     = p_tile_data_ocl->dev_last_dth_event;
	task.tile      = *p_tile;
	Ctrl_GenericEvent_Retain(task.event);
	Ctrl_TaskQueue_Push(p_ctrl->p_dth_host_stream, task);
	Ctrl_CpuEvent_Record(&p_tile_data_ocl->host_last_dth_event.event.event_cpu, p_ctrl->p_dth_host_stream);

	if (p_ctrl->policy == CTRL_POLICY_SYNC) {
		Ctrl_CpuEvent_Record(&p_ctrl->host_seq_event.event.event_cpu, p_ctrl->p_dth_host_stream);
		Ctrl_GenericEvent_Release(p_ctrl->dev_seq_event);
		p_ctrl->dev_seq_event = p_tile_data_ocl->dev_last_dth_event;
		Ctrl_GenericEvent_Retain(p_ctrl->dev_seq_event);
	}

	// Update state of the tile
	p_tile_data->host_status        = CTRL_TILE_VALID;
	p_tile_data_impl->device_status = CTRL_TILE_VALID;
}

void Ctrl_OpenCLGpu_MoveFromWait(Ctrl_OpenCL_Tile *p_tile_data, Ctrl_TaskQueue *p_queue) {
	// TODO @sergioalo does this need to wait for dth transfers?
	Ctrl_GenericEvent_StreamWait(p_tile_data->host_last_htd_event, p_queue);
	Ctrl_GenericEvent_StreamWait(p_tile_data->dev_last_htd_event, p_queue);
}

void Ctrl_OpenCLGpu_ExecTaskMoveFrom(Ctrl_Task *p_task, Ctrl_OpenCLGpu *p_ctrl) {
	HitTile          *p_tile           = &p_task->tile;
	Ctrl_Tile        *p_tile_data      = (Ctrl_Tile *)p_tile->ext;
	Ctrl_Tile_Impl   *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_OpenCL_Tile *p_tile_data_ocl  = p_tile_data_impl->tile.p_opencl;
	cl_command_queue  cmd_queue        = p_ctrl->dth_driver_stream;

	// Enqueue the transfer operation
	OpenCL_Move(Read);

	OPENCL_ASSERT_OP(clFlush(cmd_queue));
}

#undef OpenCL_Move

/**********************************
 ** TASKS'S EVALUATION FUNCTIONS **
 **********************************/

void Ctrl_OpenCLGpu_Destroy(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task) {
	#ifdef _CTRL_OPENCL_GPU_PROFILING_
	#ifdef _OPENCL_GPU_TEST_OUTPUT_
	printf("%d %d\n", p_ctrl->platform, p_ctrl->device);
	printf("%d %d %d %d %d\n", p_ctrl->i_read_task + p_ctrl->i_write_task + p_ctrl->i_kernel_task,
		   p_ctrl->i_kernel_task, p_ctrl->i_read_task + p_ctrl->i_write_task, p_ctrl->i_read_task, p_ctrl->i_write_task);
	#else
	printf("\n ----------------------- ARGS ----------------------- \n");
	printf("\n PLATFORM: %d", p_ctrl->platform);
	printf("\n DEVICE: %d", p_ctrl->device);
	printf("\n TOTAL TASKS: %d", p_ctrl->i_read_task + p_ctrl->i_write_task + p_ctrl->i_kernel_task);
	printf("\n KERNEL TASKS: %d", p_ctrl->i_kernel_task);
	printf("\n OFFLOADING TASKS: %d", p_ctrl->i_read_task + p_ctrl->i_write_task);
	printf("\n READ TASKS: %d", p_ctrl->i_read_task);
	printf("\n WRITE TASKS: %d", p_ctrl->i_write_task);
	if (p_ctrl->policy == CTRL_POLICY_ASYNC) {
		printf("\n POLICY ASYNC");
	} else {
		printf("\n POLICY SYNC");
	}
	printf("\n\n ---------------------------------------------------- \n");
	#endif // _OPENCL_GPU_TEST_OUTPUT_

	#ifdef _CTRL_OPENCL_GPU_PROFILING_VERBOSE_
	FILE *f = fopen("visual_profiler_info.txt", "w");

	for (int i = 0; i < p_ctrl->i_visual_task; i++) {
		OPENCL_ASSERT_OP(clGetEventProfilingInfo(p_ctrl->profiling_visual_events[i].event, p_ctrl->profiling_info_start, sizeof(cl_ulong), (void *)(&(p_ctrl->profiling_start)), NULL));
		OPENCL_ASSERT_OP(clGetEventProfilingInfo(p_ctrl->profiling_visual_events[i].event, p_ctrl->profiling_info_end, sizeof(cl_ulong), (void *)(&(p_ctrl->profiling_end)), NULL));

		fprintf(f, "%d; %d; %lu; %lu\n", p_ctrl->profiling_visual_events[i].queue, p_ctrl->profiling_visual_events[i].op, p_ctrl->profiling_start, p_ctrl->profiling_end);
		OPENCL_ASSERT_OP(clReleaseEvent(p_ctrl->profiling_visual_events[i].event));
	}

	fclose(f);
	free(p_ctrl->profiling_visual_events);
	#endif

	for (int i = 0; i < p_ctrl->i_read_task; i++) {
		OPENCL_ASSERT_OP(clGetEventProfilingInfo(p_ctrl->profiling_read_events[i], p_ctrl->profiling_info_start, sizeof(cl_ulong), (void *)(&(p_ctrl->profiling_start)), NULL));
		OPENCL_ASSERT_OP(clGetEventProfilingInfo(p_ctrl->profiling_read_events[i], p_ctrl->profiling_info_end, sizeof(cl_ulong), (void *)(&(p_ctrl->profiling_end)), NULL));

		p_ctrl->profiling_sum += p_ctrl->profiling_end - p_ctrl->profiling_start;
		p_ctrl->profiling_read += p_ctrl->profiling_end - p_ctrl->profiling_start;
		p_ctrl->profiling_offloading += p_ctrl->profiling_end - p_ctrl->profiling_start;

		OPENCL_ASSERT_OP(clReleaseEvent(p_ctrl->profiling_read_events[i]));
	}
	free(p_ctrl->profiling_read_events);

	for (int i = 0; i < p_ctrl->i_write_task; i++) {
		OPENCL_ASSERT_OP(clGetEventProfilingInfo(p_ctrl->profiling_write_events[i], p_ctrl->profiling_info_start, sizeof(cl_ulong), (void *)(&(p_ctrl->profiling_start)), NULL));
		OPENCL_ASSERT_OP(clGetEventProfilingInfo(p_ctrl->profiling_write_events[i], p_ctrl->profiling_info_end, sizeof(cl_ulong), (void *)(&(p_ctrl->profiling_end)), NULL));

		p_ctrl->profiling_sum += p_ctrl->profiling_end - p_ctrl->profiling_start;
		p_ctrl->profiling_write += p_ctrl->profiling_end - p_ctrl->profiling_start;
		p_ctrl->profiling_offloading += p_ctrl->profiling_end - p_ctrl->profiling_start;

		OPENCL_ASSERT_OP(clReleaseEvent(p_ctrl->profiling_write_events[i]));
	}
	free(p_ctrl->profiling_write_events);

	for (int i = 0; i < p_ctrl->i_kernel_task; i++) {
		OPENCL_ASSERT_OP(clGetEventProfilingInfo(p_ctrl->profiling_kernel_events[i], p_ctrl->profiling_info_start, sizeof(cl_ulong), (void *)(&(p_ctrl->profiling_start)), NULL));
		OPENCL_ASSERT_OP(clGetEventProfilingInfo(p_ctrl->profiling_kernel_events[i], p_ctrl->profiling_info_end, sizeof(cl_ulong), (void *)(&(p_ctrl->profiling_end)), NULL));

		p_ctrl->profiling_sum += p_ctrl->profiling_end - p_ctrl->profiling_start;
		p_ctrl->profiling_kernel += p_ctrl->profiling_end - p_ctrl->profiling_start;

		OPENCL_ASSERT_OP(clReleaseEvent(p_ctrl->profiling_kernel_events[i]));
	}
	free(p_ctrl->profiling_kernel_events);

	OPENCL_ASSERT_OP(clGetEventProfilingInfo(p_ctrl->first_profiling_event, p_ctrl->profiling_info_start, sizeof(cl_ulong), (void *)(&(p_ctrl->profiling_start)), NULL));
	OPENCL_ASSERT_OP(clGetEventProfilingInfo(p_ctrl->last_profiling_event, p_ctrl->profiling_info_end, sizeof(cl_ulong), (void *)(&(p_ctrl->profiling_end)), NULL));

	p_ctrl->profiling_total = p_ctrl->profiling_end - p_ctrl->profiling_start;

	OPENCL_ASSERT_OP(clReleaseEvent(p_ctrl->first_profiling_event));
	OPENCL_ASSERT_OP(clReleaseEvent(p_ctrl->last_profiling_event));

	#ifdef _OPENCL_GPU_TEST_OUTPUT_
	printf("%lf %lf %lf %lf %lf\n", p_ctrl->profiling_total / 1000000.0, p_ctrl->profiling_sum / 1000000.0, p_ctrl->profiling_kernel / 1000000.0, p_ctrl->profiling_offloading / 1000000.0, p_ctrl->profiling_read / 1000000.0, p_ctrl->profiling_write / 1000000.0);
	#else
	printf("\n -------------------- PROFILING --------------------- \n");
	printf("\n Total Time: %.3lf", p_ctrl->profiling_total / 1000000.0);
	printf("\n Acumulate Sum Time: %.3lf", p_ctrl->profiling_sum / 1000000.0);
	printf("\n Kernel Time: %.3lf", p_ctrl->profiling_kernel / 1000000.0);
	printf("\n Offloading Time: %.3lf", p_ctrl->profiling_offloading / 1000000.0);
	printf("\n Read Time: %.3lf", p_ctrl->profiling_read / 1000000.0);
	printf("\n Write Time: %.3lf", p_ctrl->profiling_write / 1000000.0);
	printf("\n\n ---------------------------------------------------- \n");
	#endif // _OPENCL_GPU_TEST_OUTPUT_
	#endif     // _CTRL_OPENCL_GPU_PROFILING_

	if (p_ctrl->p_tile_list_head != NULL) {
		fprintf(stderr, "Warning: Tiles left attached to ctrl %d\n", p_ctrl->global_id);
		fflush(stderr);
	}

	p_ctrl->p_tile_list_head = NULL;
	p_ctrl->p_tile_list_tail = NULL;

	// 1st ocl ctrl destroys and frees all ocl kernel stuff
	if (p_ctrl->type_id == 0) {
		for (Ctrl_OpenCLGpu_KernelParams *p_curr_kp = OpenCLGpu_initial_kp.p_next; p_curr_kp != NULL; p_curr_kp = p_curr_kp->p_next) {
			for (int i = 0; i < next_oclgpu_id; i++) {
				OPENCL_ASSERT_OP(clReleaseKernel(p_curr_kp->p_kernel[i]));
				OPENCL_ASSERT_OP(clReleaseProgram(p_curr_kp->p_program[i]));
			}
			free(p_curr_kp->p_kernel);
			free(p_curr_kp->p_program);
			free(p_curr_kp->p_kernel_raw);
		}
	}

	OPENCL_ASSERT_OP(clReleaseMemObject(p_ctrl->default_2dimg));

	Ctrl_GenericEvent_Release(p_ctrl->host_seq_event);
	Ctrl_GenericEvent_Release(p_ctrl->dev_seq_event);

	for (int i = 0; i < p_ctrl->n_kernel_streams; i++) {
		OPENCL_ASSERT_OP(clReleaseCommandQueue(p_ctrl->p_kernel_driver_streams[i]));
	}
	OPENCL_ASSERT_OP(clReleaseCommandQueue(p_ctrl->htd_driver_stream));
	OPENCL_ASSERT_OP(clReleaseCommandQueue(p_ctrl->dth_driver_stream));
	OPENCL_ASSERT_OP(clReleaseCommandQueue(p_ctrl->pin_queue));
	OPENCL_ASSERT_OP(clReleaseContext(p_ctrl->context));
	free(p_ctrl->p_kernel_driver_streams);
	free(p_ctrl->pp_kernel_host_streams);
}

void Ctrl_OpenCLGpu_EvalTaskGlobalSync(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task) {
	for (Ctrl_Tile_List *p_aux = p_ctrl->p_tile_list_head; p_aux != NULL; p_aux = p_aux->p_next) {
		Ctrl_OpenCLGpu_WaitTileInner(p_ctrl, (Ctrl_Tile *)(p_aux->p_tile_ext));
	}
}

void Ctrl_OpenCLGpu_EvalTaskKernelLaunch(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task) {
	// Check if the specified queue exists:
	if (p_task->stream < 0 || p_task->stream >= p_ctrl->n_kernel_streams) {
		fprintf(stderr, "[Ctrl_OpenCL_Gpu] Internal Error: Tried to execute a task on a nonexistent queue: %d\n", p_task->stream);
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	Ctrl_TaskQueue *p_host_kernel_queue = p_ctrl->pp_kernel_host_streams[p_task->stream];

	// wait for appropiate events from arguments according to their role
	for (int i = 0; i < p_task->n_arguments; i++) {
		if (p_task->p_roles[i] != KERNEL_INVAL) {
			HitTile          *p_tile           = (HitTile *)(p_task->pp_pointers[i]);
			Ctrl_Tile        *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
			Ctrl_Tile_Impl   *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
			Ctrl_OpenCL_Tile *p_tile_data_ocl  = p_tile_data_impl->tile.p_opencl;

			if (hit_tileIsNull(*p_tile)) {
				fprintf(stderr, "Warning: Launching task %s, skipping null tile on parameter %d (starting at 0)\n", p_task->p_func_name, i);
				fflush(stderr);
				continue;
			}

			if (p_tile_data_impl->device_memowner == HIT_MS_NOMEM) {
				fprintf(stderr, "[Ctrl_OpenCL] Internal Error: Launching kernel %s with a tile with no device memory as parameter %d (starting at 0)\n", p_task->p_func_name, i);
				fflush(stderr);
				exit(EXIT_FAILURE);
			}

			// if tile's role is IN or IO, is not updated on device and host has memory allocated transfer it
			if (p_task->p_roles[i] != KERNEL_OUT && p_tile_data_impl->device_status == CTRL_TILE_INVALID &&
				p_tile->memStatus != HIT_MS_NOMEM && p_ctrl->dependance_mode == CTRL_MODE_IMPLICIT) {
				if (p_tile_data->host_status != CTRL_TILE_VALID) {
					for (int j = 0; j < Ctrl_GetNCtrls(); j++) {
						Ctrl_Tile_Impl *p_tile_impl_j = &p_tile_data->p_impls[j];
						if (p_tile_impl_j->device_status == CTRL_TILE_VALID) {
							// TODO @sergioalo if tiles had ptr to associated abstract ctrl normal movefrom could be used and this switch removed
							switch (p_tile_impl_j->type) {
									#ifdef _CTRL_ARCH_CPU_
								case CTRL_TYPE_CPU:
									Ctrl_Cpu_EvalTaskMoveFromInner(p_tile_impl_j->tile.p_cpu->p_ctrl, p_tile);
									break;
									#endif // _CTRL_ARCH_CPU_

									#ifdef _CTRL_ARCH_CUDA_
								case CTRL_TYPE_CUDA:
									Ctrl_Cuda_EvalTaskMoveFromInner(p_tile_impl_j->tile.p_cuda->p_ctrl, p_tile);
									break;
									#endif // _CTRL_ARCH_CUDA_

									#ifdef _CTRL_ARCH_HIP_
								case CTRL_TYPE_HIP:
									Ctrl_Hip_EvalTaskMoveFromInner(p_tile_impl_j->tile.p_hip->p_ctrl, p_tile);
									break;
									#endif // _CTRL_ARCH_HIP_

									#ifdef _CTRL_ARCH_OPENCL_GPU_
								case CTRL_TYPE_OPENCL_GPU:
									Ctrl_OpenCLGpu_EvalTaskMoveFromInner(p_tile_impl_j->tile.p_opencl->p_ctrl, p_tile);
									break;
									#endif // _CTRL_ARCH_OPENCL_GPU_

									#ifdef _CTRL_ARCH_FPGA_
								case CTRL_TYPE_FPGA:
									Ctrl_FPGA_EvalTaskMoveFromInner(p_tile_impl_j->tile.p_fpga->p_ctrl, p_tile);
									break;
									#endif // _CTRL_ARCH_FPGA_
								default:
									break;
							}
							break;
						}
					}
					if (p_tile_data->host_status == CTRL_TILE_INVALID) {
						fprintf(stderr, "[Ctrl_OpenCL] Warning: Tile with uninitialized data as input on kernel %s, parameter: %d (starting at 0)\n", p_task->p_func_name, i);
						fflush(stderr);
					}
				}
				Ctrl_OpenCLGpu_EvalTaskMoveToInner(p_ctrl, p_tile);
			}

			if (p_tile_data_impl->device_status == CTRL_TILE_INVALID && p_task->p_roles[i] != KERNEL_OUT) {
				fprintf(stderr, "[Ctrl_OpenCL] Warning: Tile with uninitialized data as input on kernel %s, parameter: %d (starting at 0)\n", p_task->p_func_name, i);
				fflush(stderr);
			}

			// no need to wait for kw if last kw op was on the same stream
			if (p_tile_data_ocl->streamid_last_kw != p_task->stream) {
				Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->host_last_kernel_write_event, p_host_kernel_queue);
				Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->dev_last_kernel_write_event, p_host_kernel_queue);
			}

			if (p_tile->memStatus != HIT_MS_NOMEM && !(p_tile_data_impl->device_status == CTRL_TILE_VALID && p_tile_data->host_status == CTRL_TILE_INVALID)) {
				Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->host_last_htd_event, p_host_kernel_queue);
				Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->dev_last_htd_event, p_host_kernel_queue);
			}

			if (p_task->p_roles[i] != KERNEL_IN) {
				// no need to wait for kw if last kw op was on the same stream
				if (p_tile_data_ocl->streamid_last_kw != p_task->stream) {
					Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->host_last_kernel_read_event, p_host_kernel_queue);
					Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->dev_last_kernel_read_event, p_host_kernel_queue);
				}

				if (p_tile->memStatus != HIT_MS_NOMEM && !(p_tile_data_impl->device_status == CTRL_TILE_VALID && p_tile_data->host_status == CTRL_TILE_INVALID)) {
					Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->host_last_dth_event, p_host_kernel_queue);
					Ctrl_GenericEvent_StreamWait(p_tile_data_ocl->dev_last_dth_event, p_host_kernel_queue);
				}
			}
		}
	}

	Ctrl_SyncWait(p_host_kernel_queue);

	Ctrl_GenericEvent kernel_event = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_OPENCL, p_ctrl->global_id);

	// create request with info for kernel execution
	Ctrl_Request request;
	request.opencl.device_id           = &(p_ctrl->device_id);
	request.opencl.p_last_kernel_event = kernel_event.event.p_event_cl;
	request.opencl.n_arguments         = p_task->n_arguments;
	request.opencl.p_roles             = p_task->p_roles;
	request.opencl.p_displacements     = p_task->p_displacements;
	request.opencl.type_id             = p_ctrl->type_id;

	p_task->request = request;
	p_task->event   = kernel_event;
	Ctrl_GenericEvent_Retain(p_task->event);
	Ctrl_TaskQueue_Push(p_host_kernel_queue, *p_task);

	// update events on arguments according to their roles
	for (int i = 0; i < p_task->n_arguments; i++) {
		if (p_task->p_roles[i] != KERNEL_INVAL) {
			HitTile          *p_tile           = (HitTile *)(p_task->pp_pointers[i]);
			Ctrl_Tile        *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
			Ctrl_Tile_Impl   *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
			Ctrl_OpenCL_Tile *p_tile_data_ocl  = p_tile_data_impl->tile.p_opencl;

			Ctrl_GenericEvent_Release(p_tile_data_ocl->last_op);
			p_tile_data_ocl->last_op = kernel_event;
			Ctrl_GenericEvent_Retain(p_tile_data_ocl->last_op);

			if (p_task->p_roles[i] != KERNEL_IN) {
				if (p_tile_data->host_status == CTRL_TILE_VALID) p_tile_data->host_status = CTRL_TILE_INVALID;
				// invalidate tile on all other devs except this
				for (int j = 0; j < Ctrl_GetNCtrls(); j++) {
					Ctrl_Tile_Impl *p_tile_data_impl_j = &p_tile_data->p_impls[j];
					if (p_tile_data_impl_j->type == CTRL_TYPE_NULL) {
						continue;
					}
					if (p_tile_data_impl_j->device_status == CTRL_TILE_VALID) p_tile_data_impl_j->device_status = CTRL_TILE_INVALID;
				}

				p_tile_data_impl->device_status = CTRL_TILE_VALID;
				Ctrl_CpuEvent_Record(&p_tile_data_ocl->host_last_kernel_write_event.event.event_cpu, p_host_kernel_queue);
				Ctrl_GenericEvent_Release(p_tile_data_ocl->dev_last_kernel_write_event);
				p_tile_data_ocl->dev_last_kernel_write_event = kernel_event;
				Ctrl_GenericEvent_Retain(p_tile_data_ocl->dev_last_kernel_write_event);
				p_tile_data_ocl->streamid_last_kw = p_task->stream;
			}

			if (p_task->p_roles[i] != KERNEL_OUT) {
				Ctrl_CpuEvent_Record(&p_tile_data_ocl->host_last_kernel_read_event.event.event_cpu, p_host_kernel_queue);
				Ctrl_GenericEvent_Release(p_tile_data_ocl->dev_last_kernel_read_event);
				p_tile_data_ocl->dev_last_kernel_read_event = kernel_event;
				Ctrl_GenericEvent_Retain(p_tile_data_ocl->dev_last_kernel_read_event);
				p_tile_data_ocl->streamid_last_kr = p_task->stream;
			}
		}
	}

	if (p_ctrl->policy == CTRL_POLICY_SYNC) {
		Ctrl_CpuEvent_Record(&p_ctrl->host_seq_event.event.event_cpu, p_host_kernel_queue);
		Ctrl_GenericEvent_Release(p_ctrl->dev_seq_event);
		p_ctrl->dev_seq_event = kernel_event;
		Ctrl_GenericEvent_Retain(p_ctrl->dev_seq_event);
	}
	Ctrl_GenericEvent_Release(kernel_event);
}

void Ctrl_OpenCLGpu_HostTaskWait(Ctrl_OpenCL_Tile *p_tile, char rol, Ctrl_TaskQueue *p_queue) {
	Ctrl_GenericEvent_StreamWait(p_tile->host_last_dth_event, p_queue);
	Ctrl_GenericEvent_StreamWait(p_tile->dev_last_dth_event, p_queue);

	if (rol != KERNEL_IN) {
		Ctrl_GenericEvent_StreamWait(p_tile->host_last_htd_event, p_queue);
		Ctrl_GenericEvent_StreamWait(p_tile->dev_last_htd_event, p_queue);
	}
}

void Ctrl_OpenCLGpu_SyncWait(Ctrl_OpenCLGpu *p_ctrl, Ctrl_TaskQueue *p_queue) {
	Ctrl_GenericEvent_StreamWait(p_ctrl->host_seq_event, p_queue);
	Ctrl_GenericEvent_StreamWait(p_ctrl->dev_seq_event, p_queue);
}

bool Ctrl_OpenCLGpu_AllocPinned(Ctrl_OpenCLGpu *p_ctrl, HitTile *p_tile, int flags) {
	// don't alloc pinned mem if explicitly requested or no request made and default is to not use pinned
	if (!(flags & CTRL_MEM_PINNED) && ((flags | p_ctrl->default_alloc_mode) & CTRL_MEM_NOPINNED))
		return false;

	cl_int     err;
	Ctrl_Tile *p_tile_data = (Ctrl_Tile *)(p_tile->ext);

	p_tile_data->pinned        = CTRL_TYPE_OPENCL_GPU;
	p_tile_data->p_pinned_data = (void *)clCreateBuffer(p_ctrl->context, CL_MEM_ALLOC_HOST_PTR, (size_t)p_tile->origAcumCard[0] * p_tile->baseExtent, NULL, &err);
	OPENCL_ASSERT_ERROR(err);
	p_tile_data->p_pin_queue = (void *)p_ctrl->pin_queue;
	OPENCL_ASSERT_OP(clRetainCommandQueue(p_tile_data->p_pin_queue));
	p_tile->data = clEnqueueMapBuffer(p_ctrl->pin_queue, (cl_mem)p_tile_data->p_pinned_data, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, (size_t)p_tile->origAcumCard[0] * p_tile->baseExtent, 0, NULL, NULL, &err);
	OPENCL_ASSERT_ERROR(err);
	return true;
}

void Ctrl_OpenCLGpu_EvalTaskAllocTile(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task) {
	cl_int err;

	HitTile        *p_tile           = p_task->p_tile;
	Ctrl_Tile      *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];

	if (p_tile_data_impl->type == CTRL_TYPE_NULL)
		Ctrl_OpenCLGpu_InitTile(p_ctrl, p_task);

	Ctrl_OpenCL_Tile *p_tile_data_ocl = p_tile_data_impl->tile.p_opencl;

	if (p_task->flags & CTRL_MEM_ALLOC_DEV || !(p_task->flags & CTRL_MEM_ALLOC_HOST)) {
		if (p_tile_data_impl->device_memowner == HIT_MS_OWNER) {
			fprintf(stderr, "[Ctrl_OpenCL] Warning: This tile already owns a memory image on this device, ignoring this call.\n");
			fflush(stderr);
			return;
		}

		int dims = hit_tileDims(*p_tile);
		// Allocate device memory
		if (p_task->flags & CTRL_MEM_ALIGNED && hit_tileDims(*p_tile) > 1) {
			size_t alignment = p_ctrl->alignment;
			if (alignment == 0) {
				// allow OpenCL to choose the alignment
				cl_uint device_pitch; // In elements
				OPENCL_ASSERT_OP(clGetDeviceInfo(p_ctrl->device_id, CL_DEVICE_IMAGE_PITCH_ALIGNMENT, sizeof(cl_uint), &device_pitch, NULL));
				alignment = device_pitch * p_tile->baseExtent;
				// OpenCL device does not support image pitch alignment, fallback
				if (alignment == 0)
					alignment = CTRL_DEFAULT_TILE_ALIGNMENT;
			}
			Ctrl_Tile_UpdateOrigAcumCards(p_tile_data_impl->origAcumCard, dims, p_tile->card, alignment / p_tile->baseExtent);
			p_tile_data_ocl->device_data = clCreateBuffer(p_ctrl->context, CL_MEM_READ_WRITE, p_tile_data_impl->origAcumCard[0] * p_tile->baseExtent, NULL, &err);
			OPENCL_ASSERT_ERROR(err);

			// reject algnments not divisible by base extent
			if (alignment % p_tile->baseExtent != 0) {
				fprintf(stderr, "[Ctrl_Cuda] Error: tile alignment %lu is not divisible by type size %lu. \n", alignment, p_tile->baseExtent);
				exit(EXIT_FAILURE);
			}
		} else {
			p_tile_data_ocl->device_data = clCreateBuffer(p_ctrl->context, CL_MEM_READ_WRITE, (size_t)(p_tile->acumCard) * p_tile->baseExtent, NULL, &err);
			OPENCL_ASSERT_ERROR(err);
			Ctrl_Tile_UpdateOrigAcumCards(p_tile_data_impl->origAcumCard, dims, p_tile->card, 0);
		}

		p_tile_data_impl->device_status   = CTRL_TILE_INVALID;
		p_tile_data_impl->device_memowner = HIT_MS_OWNER;
		p_tile_data_impl->offset          = 0;
	}
}

void Ctrl_OpenCLGpu_EvalTaskSelectTile(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task) {
	HitTile *p_tile = p_task->p_tile;

	Ctrl_OpenCLGpu_InitTile(p_ctrl, p_task);

	HitTile          *p_parent           = p_tile->ref;
	Ctrl_Tile        *p_tile_data        = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl   *p_tile_data_impl   = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_OpenCL_Tile *p_tile_data_ocl    = p_tile_data_impl->tile.p_opencl;
	Ctrl_Tile        *p_parent_data      = (Ctrl_Tile *)(p_parent->ext);
	Ctrl_Tile_Impl   *p_parent_data_impl = &p_parent_data->p_impls[p_ctrl->global_id];
	Ctrl_OpenCL_Tile *p_parent_data_ocl  = p_parent_data_impl->tile.p_opencl;

	if (p_parent_data_impl->device_memowner != HIT_MS_NOMEM) {
		p_tile_data_impl->device_status   = p_parent_data_impl->device_status;
		p_tile_data_impl->device_memowner = HIT_MS_NOT_OWNER;

		for (int i = 0; i < HIT_MAXDIMS + 1; i++) {
			p_tile_data_impl->origAcumCard[i] = p_parent_data_impl->origAcumCard[i];
		}

		HitInd offset = Ctrl_Tile_ParentDeviceOffset(p_tile, p_tile_data_impl);

		// Use parent buffers. Offset added inside the kernels (device pointers can't be edited in host scope).
		p_tile_data_ocl->device_data = p_parent_data_ocl->device_data;
		p_tile_data->p_pinned_data   = p_parent_data->p_pinned_data;
		p_tile_data_impl->offset     = offset + p_parent_data_impl->offset;
	}
}

void Ctrl_OpenCLGpu_EvalTaskFreeTile(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task) {
	HitTile          *p_tile           = p_task->p_tile;
	Ctrl_Tile        *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl   *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_OpenCL_Tile *p_tile_data_ocl  = p_tile_data_impl->tile.p_opencl;

	// tile not initialized
	if (p_tile_data_impl->type == CTRL_TYPE_NULL) {
		fprintf(stderr, "[Ctrl_OpenCLGPU] Warning: Free from tile not attached to ctrl.\n");
		return;
	}

	p_tile_data->valid_impls--;

	// Wait for all work related to this tile to finish
	Ctrl_OpenCLGpu_WaitTileInner(p_ctrl, p_tile_data);

	// destroy events inside the tile
	Ctrl_GenericEvent_Release(p_tile_data_ocl->host_last_kernel_read_event);
	Ctrl_GenericEvent_Release(p_tile_data_ocl->host_last_kernel_write_event);
	Ctrl_GenericEvent_Release(p_tile_data_ocl->host_last_dth_event);
	Ctrl_GenericEvent_Release(p_tile_data_ocl->host_last_htd_event);
	Ctrl_GenericEvent_Release(p_tile_data_ocl->dev_last_kernel_read_event);
	Ctrl_GenericEvent_Release(p_tile_data_ocl->dev_last_kernel_write_event);
	Ctrl_GenericEvent_Release(p_tile_data_ocl->dev_last_dth_event);
	Ctrl_GenericEvent_Release(p_tile_data_ocl->dev_last_htd_event);
	Ctrl_GenericEvent_Release(p_tile_data_ocl->last_op);

	if (p_tile_data_impl->device_memowner == HIT_MS_OWNER) {
		OPENCL_ASSERT_OP(clReleaseMemObject(p_tile_data_ocl->device_data));
	}

	// Remove tle from tile linked list
	if (p_tile_data_ocl->p_tile_elem->p_prev != NULL) {
		p_tile_data_ocl->p_tile_elem->p_prev->p_next = p_tile_data_ocl->p_tile_elem->p_next;
	} else {
		p_ctrl->p_tile_list_head = p_tile_data_ocl->p_tile_elem->p_next;
	}

	if (p_tile_data_ocl->p_tile_elem->p_next != NULL) {
		p_tile_data_ocl->p_tile_elem->p_next->p_prev = p_tile_data_ocl->p_tile_elem->p_prev;
	} else {
		p_ctrl->p_tile_list_tail = p_tile_data_ocl->p_tile_elem->p_prev;
	}

	// Clear node fields
	p_tile_data_ocl->p_tile_elem->p_tile_ext = NULL;
	p_tile_data_ocl->p_tile_elem->p_next     = NULL;
	p_tile_data_ocl->p_tile_elem->p_prev     = NULL;

	// Free node
	free(p_tile_data_ocl->p_tile_elem);

	// Clear tile fields
	p_tile_data_ocl->p_ctrl = NULL;

	// Free tile
	free(p_tile_data_ocl);

	// if this was the last ctrl the tile was attached to free the host stuff as well
	if (p_tile_data->valid_impls == 0) {
		Ctrl_FreeHostInner(p_tile);
	}
}

void Ctrl_OpenCLGpu_EvalTaskMoveTo(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task) {
	HitTile        *p_tile           = p_task->p_tile;
	Ctrl_Tile      *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];

	if (hit_tileIsNull(*p_tile)) return;

	if (p_tile->memStatus == HIT_MS_NOMEM) {
		fprintf(stderr, "[Ctrl_OpenCL] Internal Error: Tryinng to move tile from host to device but host memory was not allocated\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	if (p_tile_data_impl->device_memowner == HIT_MS_NOMEM) {
		fprintf(stderr, "[Ctrl_OpenCL] Internal Error: Tryinng to move tile from host to device but device memory was not allocated\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	if (p_tile_data->host_status == CTRL_TILE_INVALID) {
		fprintf(stderr, "[Ctrl_OpenCL] Warning: Moving a tile from host to device with invalid data on host memory\n");
		fflush(stderr);
	}

	if (!(p_tile_data->host_status == CTRL_TILE_VALID && p_tile_data_impl->device_status == CTRL_TILE_VALID) || p_ctrl->dependance_mode == CTRL_MODE_EXPLICIT) {
		Ctrl_OpenCLGpu_EvalTaskMoveToInner(p_ctrl, p_tile);
	}
}

void Ctrl_OpenCLGpu_EvalTaskMoveFrom(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task) {
	HitTile        *p_tile           = p_task->p_tile;
	Ctrl_Tile      *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];

	if (hit_tileIsNull(*p_tile)) return;

	if (p_tile->memStatus == HIT_MS_NOMEM) {
		fprintf(stderr, "[Ctrl_OpenCL] Internal Error: Trying to move tile from device to host but host memory was not allocated\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	if (p_tile_data_impl->device_memowner == HIT_MS_NOMEM) {
		fprintf(stderr, "[Ctrl_OpenCL] Internal Error: Trying to move tile from device to host but device memory was not allocated\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	if (p_tile_data_impl->device_status == CTRL_TILE_INVALID) {
		fprintf(stderr, "[Ctrl_OpenCL] Warning: Moving a tile from device to host with invalid data on device memory\n");
		fflush(stderr);
	}

	if (!(p_tile_data->host_status == CTRL_TILE_VALID && p_tile_data_impl->device_status == CTRL_TILE_VALID) || p_ctrl->dependance_mode == CTRL_MODE_EXPLICIT) {
		Ctrl_OpenCLGpu_EvalTaskMoveFromInner(p_ctrl, p_tile);
	}
}

void Ctrl_OpenCLGpu_EvalTaskWaitTile(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task) {
	HitTile *p_tile = p_task->p_tile;

	if (hit_tileIsNull(*p_tile)) return;

	Ctrl_OpenCLGpu_WaitTileInner(p_ctrl, (Ctrl_Tile *)(p_tile->ext));
}

void Ctrl_OpenCLGpu_EvalTaskSetDependanceMode(Ctrl_OpenCLGpu *p_ctrl, Ctrl_Task *p_task) {
	p_ctrl->dependance_mode = p_task->flags;
}

///@endcond
