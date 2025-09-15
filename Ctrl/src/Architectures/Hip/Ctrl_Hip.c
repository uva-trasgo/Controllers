///@cond INTERNAL
/**
 * @file Ctrl_Hip.c
 * @brief Source code for HIP backend.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "Architectures/Hip/Ctrl_Hip.h"
#include "Core/Ctrl_Core.h"

/*************************************************************
 ******** Prototypes of private functions ********************
 *************************************************************/

/**
 * Initialize a \e Ctrl_Hip_Tile.
 *
 * @param p_ctrl Pointer to the ctrl attached to the tile and to be updated of the initialization.
 * @param p_task Pointer to the task containing the HitTile that contains the \e Ctrl_Hip_Tile to be.
 * initialized.
 */
void Ctrl_Hip_InitTile(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task);

/**
 * Waits for all the work from \p p_ctrl related to \p p_tile_data .
 *
 * @param p_ctrl Pointer to the ctrl attached to the tile.
 * @param p_tile_data Pointer to the ctrl tile.
 *
 * @see Ctrl_Hip_EvalTaskWaitTile, Ctrl_Hip_EvalTaskGlobalSync
 */
void Ctrl_Hip_WaitTileInner(Ctrl_Hip *p_ctrl, Ctrl_Tile *p_tile_data);

/**
 * Perform memory transfer from host to device.
 *
 * @param p_ctrl Pointer to the ctrl attached to the tile to be moved.
 * @param p_tile Pointer to the task containing the tile to be moved.
 *
 * @see Ctrl_Hip_EvalTaskMoveTo
 */
void Ctrl_Hip_EvalTaskMoveToInner(Ctrl_Hip *p_ctrl, HitTile *p_tile);

/**
 * Perform memory transfer from host to device.
 *
 * @param p_task Task containing the tile to be moved.
 * @param p_ctrl Ctrl responsible for this task
 *
 * @see Ctrl_Cuda_EvalTaskMoveTo
 */
void Ctrl_Hip_ExecTaskMoveTo(Ctrl_Task *p_task, Ctrl_Hip *p_ctrl);

/**
 * Perform memory transfer from device to host.
 *
 * @param p_ctrl Pointer to the ctrl attached to the tile to be moved.
 * @param p_tile Pointer to the task containing the tile to be moved.
 *
 * @see Ctrl_Hip_EvalTaskMoveFrom
 */
void Ctrl_Hip_EvalTaskMoveFromInner(Ctrl_Hip *p_ctrl, HitTile *p_tile);

/**
 * Perform memory transfer from device to host.
 *
 * @param p_task Task containing the tile to be moved.
 * @param p_ctrl Ctrl responsible for this task
 *
 * @see Ctrl_Hip_EvalTaskMoveFrom
 */
void Ctrl_Hip_ExecTaskMoveFrom(Ctrl_Task *p_task, Ctrl_Hip *p_ctrl);

/**
 * Get \p qid th driver queue used by \p p_ctrl
 * Order of qid is htd, dth, kernels.
 *
 * @param p_ctrl Ctrl to get the queues from.
 * @param qid Pointer to the driver queues used by this ctrl
 * @return Stream \p qid of ctrl \p p_ctrl
 *
 * @pre \p qid must be between 0 and the amount of queues of \p p_ctrl
 * @see Ctrl_Hip_GetNumQueues
 */
hipStream_t Ctrl_Hip_GetStreamById(Ctrl_Hip *p_ctrl, int qid);

/*************************************************************
 ******** Prototypes of tasks' evaluation functions **********
 *************************************************************/

/**
 * Destroy a HIP ctrl.
 * @param p_ctrl ctrl to be destroyed.
 * @param p_task Task with info for cleanup
 */
void Ctrl_Hip_Destroy(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of kernel launch.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_Hip_EvalTask, Ctrl_Launch
 */
void Ctrl_Hip_EvalTaskKernelLaunch(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of global sync. Waits for all work related to any tile attached to this ctrl.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_Hip_EvalTask, Ctrl_GlobalSync
 */
void Ctrl_Hip_EvalTaskGlobalSync(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of allocation of tiles.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_Hip_EvalTask, Ctrl_Alloc
 */
void Ctrl_Hip_EvalTaskAllocTile(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of subselecting tiles.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_Hip_EvalTask, Ctrl_Select
 */
void Ctrl_Hip_EvalTaskSelectTile(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of freeing of tiles.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_Hip_EvalTask, Ctrl_Free
 */
void Ctrl_Hip_EvalTaskFreeTile(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of comunication of tile from host to device.
 *
 * This function checks state of tile and, if comunication is necesary calls to \e Ctrl_Hip_EvalTaskMoveToInner.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_Hip_EvalTask, Ctrl_Hip_EvalTaskMoveToInner, Ctrl_MoveTo
 */
void Ctrl_Hip_EvalTaskMoveTo(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of comunication of tile from host to device.
 *
 * This function checks state of tile and, if comunication is necesary calls to \e Ctrl_Hip_EvalTaskMoveToInner.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_Hip_EvalTask, Ctrl_Hip_EvalTaskMoveToInner, Ctrl_MoveTo
 */
void Ctrl_Hip_EvalTaskMoveFrom(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of wait. Waits for all the work related to the tile specified in \p p_task .
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated.
 *
 * @see Ctrl_Hip_EvalTask, Ctrl_WaitTile
 */
void Ctrl_Hip_EvalTaskWaitTile(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task);

/**
 * Evaluation of change of dependance mode.
 *
 * @param p_ctrl Ctrl in charge of task.
 * @param p_task Task to be evaluated. .
 */
void Ctrl_Hip_EvalTaskSetDependanceMode(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task);

/********************************************
 ******** HIP Controller functions **********
 ********************************************/

void Ctrl_Hip_Create(Ctrl_Hip *p_ctrl, Ctrl_Policy policy, char *args) {
	p_ctrl->policy           = policy;
	p_ctrl->device           = atoi(strtok(args, " "));
	char *streams            = strtok(NULL, "");
	p_ctrl->n_kernel_streams = streams == NULL ? 1 : atoi(streams);
	if (p_ctrl->n_kernel_streams <= 0) {
		p_ctrl->n_kernel_streams = 1;
		fprintf(stderr, "[Ctrl_Hip] Warning: Tried to create Hip Ctrl with less than one stream; defaulting to 1.");
		fflush(stderr);
	}
	p_ctrl->p_kernel_driver_streams = (hipStream_t *)malloc(p_ctrl->n_kernel_streams * sizeof(hipStream_t));
	p_ctrl->pp_kernel_host_streams  = (Ctrl_TaskQueue **)malloc(p_ctrl->n_kernel_streams * sizeof(Ctrl_TaskQueue *));
	p_ctrl->dependance_mode         = CTRL_MODE_IMPLICIT;
	p_ctrl->default_alloc_mode      = CTRL_MEM_PINNED;

	// set the main thread to use the desired Hip device.
	HIP_OP(hipSetDevice(p_ctrl->device));

	p_ctrl->p_tile_list_head = NULL;
	p_ctrl->p_tile_list_tail = NULL;

	// Init streams
	HIP_OP(hipStreamCreateWithFlags(&(p_ctrl->htd_driver_stream), hipStreamNonBlocking));
	HIP_OP(hipStreamCreateWithFlags(&(p_ctrl->dth_driver_stream), hipStreamNonBlocking));
	p_ctrl->p_htd_host_stream = Ctrl_TaskQueue_Create();
	p_ctrl->p_dth_host_stream = Ctrl_TaskQueue_Create();
	for (int i = 0; i < p_ctrl->n_kernel_streams; i++) {
		HIP_OP(hipStreamCreateWithFlags(&(p_ctrl->p_kernel_driver_streams[i]), hipStreamNonBlocking));
		p_ctrl->pp_kernel_host_streams[i] = Ctrl_TaskQueue_Create();
	}

	p_ctrl->host_seq_event = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_CPU, p_ctrl->global_id);
	p_ctrl->dev_seq_event  = CTRL_GENERIC_EVENT_NULL;

	#ifdef _CTRL_HIPBLAS_
	hipblasCreate(&(p_ctrl->hipblas_handle));
	hipblasSetStream(p_ctrl->hipblas_handle, p_ctrl->p_kernel_driver_streams[0]);
	#endif // _CTRL_HIPBLAS_
}

void Ctrl_Hip_EvalTask(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task) {
	switch (p_task->task_type) {
		case CTRL_TASK_TYPE_KERNEL:
			Ctrl_Hip_EvalTaskKernelLaunch(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_GLOBALSYNC:
			Ctrl_Hip_EvalTaskGlobalSync(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_ALLOCTILE:
			Ctrl_Hip_EvalTaskAllocTile(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_SELECTTILE:
			Ctrl_Hip_EvalTaskSelectTile(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_FREETILE:
			Ctrl_Hip_EvalTaskFreeTile(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_MOVETO:
			Ctrl_Hip_EvalTaskMoveTo(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_MOVEFROM:
			Ctrl_Hip_EvalTaskMoveFrom(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_WAITTILE:
			Ctrl_Hip_EvalTaskWaitTile(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_DESTROYCTRL:
			Ctrl_Hip_Destroy(p_ctrl, p_task);
			break;
		case CTRL_TASK_TYPE_SETDEPENDANCEMODE:
			Ctrl_Hip_EvalTaskSetDependanceMode(p_ctrl, p_task);
			break;
		default:
			fprintf(stderr, "[Ctrl_Hip] Unsupported task type: %d.\n", p_task->task_type);
			fflush(stderr);
			exit(EXIT_FAILURE);
	}
}

void Ctrl_Hip_ExecTask(Ctrl_Task *p_task, Ctrl_Hip *p_ctrl) {
	HIP_OP(hipSetDevice(p_ctrl->device));

	hipStream_t stream = Ctrl_Hip_GetStreamById(p_ctrl, p_task->stream);

	p_task->request.hip.p_stream = &stream;
	switch (p_task->task_type) {
		case CTRL_TASK_TYPE_KERNEL:
			p_task->pfn_kernel_wrapper(p_task->request, p_task->device_id, CTRL_TYPE_HIP, p_task->threads, p_task->blocksize, p_task->p_arguments);
			// skip creating/recording event if this is the last reference to it
			if (Ctrl_GenericEvent_GetRefCount(p_task->event) > 1) {
				HIP_OP(hipEventCreateWithFlags(p_task->event.event.p_event_hip, hipEventDisableTiming));
				HIP_OP(hipEventRecord(*p_task->event.event.p_event_hip, stream));
				Ctrl_GenericEvent_Release(p_task->event);
			}
			break;
		case CTRL_TASK_TYPE_MOVETO:
			Ctrl_Hip_ExecTaskMoveTo(p_task, p_ctrl);
			// skip creating/recording event if this is the last reference to it
			if (Ctrl_GenericEvent_GetRefCount(p_task->event) > 1) {
				HIP_OP(hipEventCreateWithFlags(p_task->event.event.p_event_hip, hipEventDisableTiming));
				HIP_OP(hipEventRecord(*p_task->event.event.p_event_hip, stream));
				Ctrl_GenericEvent_Release(p_task->event);
			}
			break;
		case CTRL_TASK_TYPE_MOVEFROM:
			Ctrl_Hip_ExecTaskMoveFrom(p_task, p_ctrl);
			// skip creating/recording event if this is the last reference to it
			if (Ctrl_GenericEvent_GetRefCount(p_task->event) > 1) {
				HIP_OP(hipEventCreateWithFlags(p_task->event.event.p_event_hip, hipEventDisableTiming));
				HIP_OP(hipEventRecord(*p_task->event.event.p_event_hip, stream));
				Ctrl_GenericEvent_Release(p_task->event);
			}
			break;
		case CTRL_TASK_TYPE_WAITEVENT:
			HIP_OP(hipStreamWaitEvent(stream, *p_task->event.event.p_event_hip, 0));
			break;
		default:
			fprintf(stderr, "[Ctrl_Hip] ExecTask: task type %d should not get here\n", p_task->task_type);
			exit(EXIT_FAILURE);
	}
}

int Ctrl_Hip_GetNumQueues(Ctrl_Hip *p_ctrl) {
	return p_ctrl->n_kernel_streams + 2;
}

Ctrl_TaskQueue **Ctrl_Hip_GetHostQueues(Ctrl_Hip *p_ctrl, Ctrl_TaskQueue **pp_queues) {
	pp_queues[0] = p_ctrl->p_htd_host_stream;
	pp_queues[1] = p_ctrl->p_dth_host_stream;

	for (int i = 0; i < p_ctrl->n_kernel_streams; i++) {
		pp_queues[i + 2] = p_ctrl->pp_kernel_host_streams[i];
	}

	return &pp_queues[Ctrl_Hip_GetNumQueues(p_ctrl)];
}

hipStream_t Ctrl_Hip_GetStreamById(Ctrl_Hip *p_ctrl, int qid) {
	if (qid < 0 || qid > Ctrl_Hip_GetNumQueues(p_ctrl)) {
		fprintf(stderr, "[Ctrl_Hip_GetStreamById] Invalid qid %d\n", qid);
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

void Ctrl_Hip_GetInfo(Ctrl_Hip *p_ctrl, Ctrl_Info *p_info) {
	hipDeviceProp_t hip_dev_prop;
	hipGetDeviceProperties(&hip_dev_prop, p_ctrl->device);
	p_info->type = "HIP";
	strncpy(p_info->device_name, hip_dev_prop.name, CTRL_MAX_DEV_NAME - 1);
	p_info->device_name[255] = '\0';
	p_info->n_kernel_queues  = p_ctrl->n_kernel_streams;
}

void Ctrl_Hip_CreateTex(Ctrl_Hip *p_ctrl, HitTile *p_tile, Ctrl_TexDesc tex_desc) {
	HIP_OP(hipSetDevice(p_ctrl->device));

	Ctrl_Tile      *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_Hip_Tile  *p_tile_data_hip  = p_tile_data_impl->tile.p_hip;

	if (hit_tileDims(*p_tile) != 2) {
		fprintf(stderr, "[Ctrl_Hip_CreateTex] Error: textures are currently only supported for 2D matrixes.");
		exit(EXIT_FAILURE);
	}

	if (p_tile_data_impl->type == CTRL_TYPE_NULL) {
		fprintf(stderr, "[Ctrl_Hip_CreateTex] Error: tile not asociated with ctrl.");
		exit(EXIT_FAILURE);
	}

	// TODO @sergioalo check if pitch is valid for texture

	if (p_tile_data_impl->device_status == CTRL_TILE_UNALLOC) {
		fprintf(stderr, "[Ctrl_Hip_CreateTex] Error: Device memory not allocated for this tile. You must allocate device memory before creating textures.\n");
		exit(EXIT_FAILURE);
	}

	// TODO @sergioalo channel descriptor stuff should not be hardcoded
	// TODO @sergioalo overriden texture objects are not destroyed, but this is also not done on hip samples

	// Create texture object
	struct hipResourceDesc resDesc;
	memset(&resDesc, 0, sizeof(resDesc));
	resDesc.resType                  = hipResourceTypePitch2D;
	resDesc.res.pitch2D.devPtr       = p_tile_data_hip->p_device_data;
	resDesc.res.pitch2D.width        = tex_desc.width == 0 ? hit_tileDimCard(*p_tile, 1) : tex_desc.width;
	resDesc.res.pitch2D.height       = tex_desc.height == 0 ? hit_tileDimCard(*p_tile, 0) : tex_desc.height;
	resDesc.res.pitch2D.desc         = hipCreateChannelDesc(32, 0, 0, 0, hipChannelFormatKindFloat);
	resDesc.res.pitch2D.pitchInBytes = p_tile_data_hip->pitch;
	struct hipTextureDesc texDesc;
	memset(&texDesc, 0, sizeof(texDesc));
	texDesc.normalizedCoords = tex_desc.normalized_coords;
	texDesc.filterMode       = tex_desc.filter_mode;
	texDesc.addressMode[0]   = tex_desc.addr_mode[0];
	texDesc.addressMode[1]   = tex_desc.addr_mode[1];
	texDesc.addressMode[2]   = tex_desc.addr_mode[2];
	texDesc.readMode         = tex_desc.read_mode;
	HIP_OP(hipCreateTextureObject(&p_tile_data_hip->texture, &resDesc, &texDesc, NULL));
}

void *Ctrl_Hip_GetDevPtr(Ctrl_Hip *p_ctrl, HitTile *p_tile) {
	Ctrl_Tile      *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_Hip_Tile  *p_tile_data_hip  = p_tile_data_impl->tile.p_hip;

	if (p_tile_data_hip == NULL || p_tile_data_impl->device_status == CTRL_TILE_UNALLOC) return NULL;

	return p_tile_data_hip->p_device_data;
}

/*********************************
 ******* Private functions *******
 *********************************/

void Ctrl_Hip_InitTile(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task) {
	Ctrl_Tile *p_tile_data = (Ctrl_Tile *)(p_task->p_tile->ext);
	p_tile_data->valid_impls++;

	Ctrl_Hip_Tile *p_tile_data_impl_hip                = (Ctrl_Hip_Tile *)malloc(sizeof(Ctrl_Hip_Tile));
	p_tile_data->p_impls[p_ctrl->global_id].type       = CTRL_TYPE_HIP;
	p_tile_data->p_impls[p_ctrl->global_id].tile.p_hip = p_tile_data_impl_hip;

	p_tile_data_impl_hip->p_ctrl  = p_ctrl;
	p_tile_data_impl_hip->pitch   = 0;
	p_tile_data_impl_hip->texture = 0;

	Ctrl_Tile_List *p_list_node = (Ctrl_Tile_List *)malloc(sizeof(Ctrl_Tile_List));

	p_list_node->p_prev               = NULL;
	p_list_node->p_next               = NULL;
	p_list_node->p_tile_ext           = p_tile_data;
	p_tile_data_impl_hip->p_tile_elem = p_list_node;

	if (p_ctrl->p_tile_list_tail != NULL) {
		p_ctrl->p_tile_list_tail->p_next = p_list_node;
		p_list_node->p_prev              = p_ctrl->p_tile_list_tail;
		p_ctrl->p_tile_list_tail         = p_list_node;
	} else {
		p_ctrl->p_tile_list_head = p_ctrl->p_tile_list_tail = p_list_node;
	}

	p_tile_data_impl_hip->host_last_kernel_read_event  = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_CPU, p_ctrl->global_id);
	p_tile_data_impl_hip->host_last_kernel_write_event = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_CPU, p_ctrl->global_id);
	p_tile_data_impl_hip->host_last_dth_event          = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_CPU, p_ctrl->global_id);
	p_tile_data_impl_hip->host_last_htd_event          = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_CPU, p_ctrl->global_id);
	p_tile_data_impl_hip->dev_last_kernel_read_event   = CTRL_GENERIC_EVENT_NULL;
	p_tile_data_impl_hip->dev_last_kernel_write_event  = CTRL_GENERIC_EVENT_NULL;
	p_tile_data_impl_hip->dev_last_dth_event           = CTRL_GENERIC_EVENT_NULL;
	p_tile_data_impl_hip->dev_last_htd_event           = CTRL_GENERIC_EVENT_NULL;

	p_tile_data_impl_hip->streamid_last_kr = 0;
	p_tile_data_impl_hip->streamid_last_kw = 0;
}

void Ctrl_Hip_WaitTileInner(Ctrl_Hip *p_ctrl, Ctrl_Tile *p_tile_data) {
	Ctrl_Hip_Tile *p_tile_data_hip = p_tile_data->p_impls[p_ctrl->global_id].tile.p_hip;

	// Wait for all work related to this tile to finish
	Ctrl_GenericEvent_Wait(p_tile_data->last_host_read_event);
	Ctrl_GenericEvent_Wait(p_tile_data->last_host_write_event);
	Ctrl_GenericEvent_Wait(p_tile_data_hip->host_last_kernel_read_event);
	Ctrl_GenericEvent_Wait(p_tile_data_hip->host_last_kernel_write_event);
	Ctrl_GenericEvent_Wait(p_tile_data_hip->host_last_dth_event);
	Ctrl_GenericEvent_Wait(p_tile_data_hip->host_last_htd_event);
	Ctrl_GenericEvent_Wait(p_tile_data_hip->dev_last_kernel_read_event);
	Ctrl_GenericEvent_Wait(p_tile_data_hip->dev_last_kernel_write_event);
	Ctrl_GenericEvent_Wait(p_tile_data_hip->dev_last_dth_event);
	Ctrl_GenericEvent_Wait(p_tile_data_hip->dev_last_htd_event);

	Ctrl_GenericEvent_Release(p_tile_data_hip->dev_last_kernel_read_event);
	Ctrl_GenericEvent_Release(p_tile_data_hip->dev_last_kernel_write_event);
	Ctrl_GenericEvent_Release(p_tile_data_hip->dev_last_dth_event);
	Ctrl_GenericEvent_Release(p_tile_data_hip->dev_last_htd_event);

	p_tile_data_hip->dev_last_kernel_read_event  = CTRL_GENERIC_EVENT_NULL;
	p_tile_data_hip->dev_last_kernel_write_event = CTRL_GENERIC_EVENT_NULL;
	p_tile_data_hip->dev_last_dth_event          = CTRL_GENERIC_EVENT_NULL;
	p_tile_data_hip->dev_last_htd_event          = CTRL_GENERIC_EVENT_NULL;
}

void Ctrl_Hip_EvalTaskMoveToInner(Ctrl_Hip *p_ctrl, HitTile *p_tile) {
	Ctrl_Tile      *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_Hip_Tile  *p_tile_data_hip  = p_tile_data_impl->tile.p_hip;

	// Wait for appropiate events
	HIP_OP(hipSetDevice(p_ctrl->device));

	for (int i = 0; i < Ctrl_GetNCtrls(); i++) {
		if (i == p_ctrl->global_id) continue;

		Ctrl_MoveToWait(&p_tile_data->p_impls[i], p_ctrl->p_htd_host_stream);
	}

	Ctrl_GenericEvent_StreamWait(p_tile_data_hip->host_last_kernel_read_event, p_ctrl->p_htd_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data_hip->host_last_kernel_write_event, p_ctrl->p_htd_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data->last_host_write_event, p_ctrl->p_htd_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data_hip->dev_last_kernel_read_event, p_ctrl->p_htd_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data_hip->dev_last_kernel_write_event, p_ctrl->p_htd_host_stream);

	// Wait for previous task to finish if policy is sync
	Ctrl_SyncWait(p_ctrl->p_htd_host_stream);

	Ctrl_GenericEvent_Release(p_tile_data_hip->dev_last_htd_event);
	p_tile_data_hip->dev_last_htd_event = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_HIP, p_ctrl->global_id);

	Ctrl_Task task = CTRL_TASK_NULL;
	task.task_type = CTRL_TASK_TYPE_MOVETO;
	task.event     = p_tile_data_hip->dev_last_htd_event;
	task.tile      = *p_tile;
	Ctrl_GenericEvent_Retain(task.event);
	Ctrl_TaskQueue_Push(p_ctrl->p_htd_host_stream, task);
	Ctrl_CpuEvent_Record(&p_tile_data_hip->host_last_htd_event.event.event_cpu, p_ctrl->p_htd_host_stream);

	if (p_ctrl->policy == CTRL_POLICY_SYNC) {
		Ctrl_CpuEvent_Record(&p_ctrl->host_seq_event.event.event_cpu, p_ctrl->p_dth_host_stream);
		Ctrl_GenericEvent_Release(p_ctrl->dev_seq_event);
		p_ctrl->dev_seq_event = p_tile_data_hip->dev_last_htd_event;
		Ctrl_GenericEvent_Retain(p_ctrl->dev_seq_event);
	}

	// Update state of the tile
	p_tile_data->host_status        = CTRL_TILE_VALID;
	p_tile_data_impl->device_status = CTRL_TILE_VALID;
}

void Ctrl_Hip_MoveToWait(Ctrl_Hip_Tile *p_tile_data, Ctrl_TaskQueue *p_queue) {
	Ctrl_GenericEvent_StreamWait(p_tile_data->host_last_dth_event, p_queue);
	Ctrl_GenericEvent_StreamWait(p_tile_data->dev_last_dth_event, p_queue);
}

void Ctrl_Hip_ExecTaskMoveTo(Ctrl_Task *p_task, Ctrl_Hip *p_ctrl) {
	HitTile       *p_tile          = &p_task->tile;
	Ctrl_Tile     *p_tile_data     = (Ctrl_Tile *)p_tile->ext;
	Ctrl_Hip_Tile *p_tile_data_hip = p_tile_data->p_impls[p_ctrl->global_id].tile.p_hip;

	HitTile flat_tile = *p_tile;
	size_t  pitch     = p_tile_data_hip->pitch;
	if (pitch == 0) {
		hit_tileFlattenDims(&flat_tile);
		pitch = (flat_tile.baseExtent) * flat_tile.origAcumCard[1];
	}

	switch (hit_tileDims(flat_tile)) {
		case 1:
			HIP_OP(
				hipMemcpyAsync(p_tile_data_hip->p_device_data,
							   flat_tile.data,
							   (size_t)flat_tile.acumCard * flat_tile.baseExtent,
							   hipMemcpyHostToDevice,
							   p_ctrl->htd_driver_stream));
			break;
		case 2:
			HIP_OP(
				hipMemcpy2DAsync(p_tile_data_hip->p_device_data,
								 pitch,
								 flat_tile.data,
								 flat_tile.baseExtent * flat_tile.origAcumCard[1],
								 flat_tile.baseExtent * flat_tile.card[1],
								 flat_tile.card[0],
								 hipMemcpyHostToDevice,
								 p_ctrl->htd_driver_stream));
			break;
		case 3: {
			struct hipMemcpy3DParms params = {0};

			params.srcPtr = make_hipPitchedPtr(flat_tile.data,
											   flat_tile.baseExtent * flat_tile.origAcumCard[2],
											   flat_tile.baseExtent * flat_tile.origAcumCard[2],
											   flat_tile.origAcumCard[1] / flat_tile.origAcumCard[2]);
			params.dstPtr = make_hipPitchedPtr(p_tile_data_hip->p_device_data,
											   flat_tile.baseExtent * flat_tile.origAcumCard[2],
											   flat_tile.baseExtent * flat_tile.origAcumCard[2],
											   flat_tile.origAcumCard[1] / flat_tile.origAcumCard[2]);
			params.extent = make_hipExtent(flat_tile.card[2] * flat_tile.baseExtent, flat_tile.card[1], flat_tile.card[0]);
			params.kind   = hipMemcpyHostToDevice;

			HIP_OP(hipMemcpy3DAsync(&params, p_ctrl->htd_driver_stream));
			break;
		}
		case 4:
			for (int i = 0; i < hit_tileDimCard(flat_tile, 0); i++) {
				size_t matrix_offset = flat_tile.baseExtent * i * flat_tile.origAcumCard[1];

				struct hipMemcpy3DParms params = {0};

				params.srcPtr = make_hipPitchedPtr((void *)((char *)flat_tile.data + matrix_offset),
												   flat_tile.baseExtent * flat_tile.origAcumCard[3],
												   flat_tile.baseExtent * flat_tile.origAcumCard[3],
												   flat_tile.origAcumCard[2] / flat_tile.origAcumCard[3]);
				params.dstPtr = make_hipPitchedPtr((void *)((char *)p_tile_data_hip->p_device_data + matrix_offset),
												   flat_tile.baseExtent * flat_tile.origAcumCard[3],
												   flat_tile.baseExtent * flat_tile.origAcumCard[3],
												   flat_tile.origAcumCard[2] / flat_tile.origAcumCard[3]);
				params.extent = make_hipExtent(flat_tile.card[3] * flat_tile.baseExtent, flat_tile.card[2], flat_tile.card[1]);
				params.kind   = hipMemcpyHostToDevice;

				HIP_OP(hipMemcpy3DAsync(&params, p_ctrl->htd_driver_stream));
			}
			break;

		default:
			fprintf(stderr, "Internal Error: Number of dimensions not supported for non-owner tile in MoveFrom: %d\n", hit_tileDims(flat_tile));
			exit(EXIT_FAILURE);
			break;
	}
}

void Ctrl_Hip_EvalTaskMoveFromInner(Ctrl_Hip *p_ctrl, HitTile *p_tile) {
	Ctrl_Tile      *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_Hip_Tile  *p_tile_data_hip  = p_tile_data_impl->tile.p_hip;

	// Wait for appropiate events
	HIP_OP(hipSetDevice(p_ctrl->device));

	for (int i = 0; i < Ctrl_GetNCtrls(); i++) {
		if (i == p_ctrl->global_id) continue;

		Ctrl_MoveFromWait(&p_tile_data->p_impls[i], p_ctrl->p_dth_host_stream);
	}

	Ctrl_GenericEvent_StreamWait(p_tile_data_hip->host_last_kernel_write_event, p_ctrl->p_dth_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data->last_host_read_event, p_ctrl->p_dth_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data->last_host_write_event, p_ctrl->p_dth_host_stream);
	Ctrl_GenericEvent_StreamWait(p_tile_data_hip->dev_last_kernel_write_event, p_ctrl->p_dth_host_stream);

	// wait for previous task to finish if policy is sync
	Ctrl_SyncWait(p_ctrl->p_dth_host_stream);

	Ctrl_GenericEvent_Release(p_tile_data_hip->dev_last_dth_event);
	p_tile_data_hip->dev_last_dth_event = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_HIP, p_ctrl->global_id);

	Ctrl_Task task = CTRL_TASK_NULL;
	task.task_type = CTRL_TASK_TYPE_MOVEFROM;
	task.event     = p_tile_data_hip->dev_last_dth_event;
	task.tile      = *p_tile;
	Ctrl_GenericEvent_Retain(task.event);
	Ctrl_TaskQueue_Push(p_ctrl->p_dth_host_stream, task);
	Ctrl_CpuEvent_Record(&p_tile_data_hip->host_last_dth_event.event.event_cpu, p_ctrl->p_dth_host_stream);

	if (p_ctrl->policy == CTRL_POLICY_SYNC) {
		Ctrl_CpuEvent_Record(&p_ctrl->host_seq_event.event.event_cpu, p_ctrl->p_htd_host_stream);
		Ctrl_GenericEvent_Release(p_ctrl->dev_seq_event);
		p_ctrl->dev_seq_event = p_tile_data_hip->dev_last_dth_event;
		Ctrl_GenericEvent_Retain(p_ctrl->dev_seq_event);
	}

	// Update state of the tile
	p_tile_data->host_status        = CTRL_TILE_VALID;
	p_tile_data_impl->device_status = CTRL_TILE_VALID;
}

void Ctrl_Hip_MoveFromWait(Ctrl_Hip_Tile *p_tile_data, Ctrl_TaskQueue *p_queue) {
	// TODO @sergioalo does this need to wait for dth transfers?
	Ctrl_GenericEvent_StreamWait(p_tile_data->host_last_htd_event, p_queue);
	Ctrl_GenericEvent_StreamWait(p_tile_data->dev_last_htd_event, p_queue);
}

void Ctrl_Hip_ExecTaskMoveFrom(Ctrl_Task *p_task, Ctrl_Hip *p_ctrl) {
	HitTile       *p_tile          = &p_task->tile;
	Ctrl_Tile     *p_tile_data     = (Ctrl_Tile *)p_tile->ext;
	Ctrl_Hip_Tile *p_tile_data_hip = p_tile_data->p_impls[p_ctrl->global_id].tile.p_hip;

	HitTile flat_tile = *p_tile;
	size_t  pitch     = p_tile_data_hip->pitch;
	if (pitch == 0) {
		hit_tileFlattenDims(&flat_tile);
		pitch = (flat_tile.baseExtent) * flat_tile.origAcumCard[1];
	}

	switch (hit_tileDims(flat_tile)) {
		case 1:
			HIP_OP(
				hipMemcpyAsync(flat_tile.data,
							   p_tile_data_hip->p_device_data,
							   (size_t)flat_tile.acumCard * flat_tile.baseExtent,
							   hipMemcpyDeviceToHost,
							   p_ctrl->dth_driver_stream));
			break;
		case 2:
			HIP_OP(
				hipMemcpy2DAsync(flat_tile.data,                                   // dst
								 flat_tile.baseExtent * flat_tile.origAcumCard[1], // dpitch
								 p_tile_data_hip->p_device_data,                   // src
								 pitch,                                            // spitch
								 flat_tile.baseExtent * flat_tile.card[1],         // width
								 flat_tile.card[0],                                // height
								 hipMemcpyDeviceToHost,
								 p_ctrl->dth_driver_stream));
			break;
		case 3: {
			struct hipMemcpy3DParms params = {0};

			params.srcPtr = make_hipPitchedPtr(p_tile_data_hip->p_device_data,
											   flat_tile.baseExtent * flat_tile.origAcumCard[2],
											   flat_tile.baseExtent * flat_tile.origAcumCard[2],
											   flat_tile.origAcumCard[1] / flat_tile.origAcumCard[2]);
			params.dstPtr = make_hipPitchedPtr(flat_tile.data,
											   flat_tile.baseExtent * flat_tile.origAcumCard[2],
											   flat_tile.baseExtent * flat_tile.origAcumCard[2],
											   flat_tile.origAcumCard[1] / flat_tile.origAcumCard[2]);
			params.extent = make_hipExtent(flat_tile.card[2] * flat_tile.baseExtent, flat_tile.card[1], flat_tile.card[0]);
			params.kind   = hipMemcpyDeviceToHost;

			HIP_OP(hipMemcpy3DAsync(&params, p_ctrl->dth_driver_stream));
			break;
		}
		case 4:
			for (int i = 0; i < hit_tileDimCard(flat_tile, 0); i++) {
				size_t matrix_offset = flat_tile.baseExtent * i * flat_tile.origAcumCard[1];

				struct hipMemcpy3DParms params = {0};

				params.srcPtr = make_hipPitchedPtr((void *)((char *)p_tile_data_hip->p_device_data + matrix_offset),
												   flat_tile.baseExtent * flat_tile.origAcumCard[3],
												   flat_tile.baseExtent * flat_tile.origAcumCard[3],
												   flat_tile.origAcumCard[2] / flat_tile.origAcumCard[3]);
				params.dstPtr = make_hipPitchedPtr((void *)((char *)flat_tile.data + matrix_offset),
												   flat_tile.baseExtent * flat_tile.origAcumCard[3],
												   flat_tile.baseExtent * flat_tile.origAcumCard[3],
												   flat_tile.origAcumCard[2] / flat_tile.origAcumCard[3]);
				params.extent = make_hipExtent(flat_tile.card[3] * flat_tile.baseExtent, flat_tile.card[2], flat_tile.card[1]);
				params.kind   = hipMemcpyDeviceToHost;

				HIP_OP(hipMemcpy3DAsync(&params, p_ctrl->dth_driver_stream));
			}
			break;

		default:
			fprintf(stderr, "Internal Error: Number of dimensions not supported for non-owner tile in MoveFrom: %d\n", hit_tileDims(flat_tile));
			exit(EXIT_FAILURE);
			break;
	}
}

/**********************************
 ** TASKS'S EVALUATION FUNCTIONS **
 **********************************/

void Ctrl_Hip_Destroy(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task) {
	if (p_ctrl->p_tile_list_head != NULL) {
		fprintf(stderr, "Warning: Tiles left attached to ctrl %d\n", p_ctrl->global_id);
		fflush(stderr);
	}

	p_ctrl->p_tile_list_head = NULL;
	p_ctrl->p_tile_list_tail = NULL;

	for (int i = 0; i < p_ctrl->n_kernel_streams; i++) {
		HIP_OP(hipStreamDestroy(p_ctrl->p_kernel_driver_streams[i]));
	}
	HIP_OP(hipStreamDestroy(p_ctrl->htd_driver_stream));
	HIP_OP(hipStreamDestroy(p_ctrl->dth_driver_stream));

	Ctrl_GenericEvent_Release(p_ctrl->host_seq_event);
	Ctrl_GenericEvent_Release(p_ctrl->dev_seq_event);

	free(p_ctrl->p_kernel_driver_streams);
	free(p_ctrl->pp_kernel_host_streams);

	#ifdef _CTRL_HIPBLAS_
	hipblasDestroy(p_ctrl->hipblas_handle);
	#endif // _CTRL_HIPBLAS_
}

void Ctrl_Hip_EvalTaskGlobalSync(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task) {
	for (Ctrl_Tile_List *p_aux = p_ctrl->p_tile_list_head; p_aux != NULL; p_aux = p_aux->p_next) {
		Ctrl_Hip_WaitTileInner(p_ctrl, (Ctrl_Tile *)(p_aux->p_tile_ext));
	}
}

void Ctrl_Hip_EvalTaskKernelLaunch(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task) {
	HIP_OP(hipSetDevice(p_ctrl->device));

	// Check if the specified stream exists:
	if (p_task->stream < 0 || p_task->stream >= p_ctrl->n_kernel_streams) {
		fprintf(stderr, "[Ctrl_Hip] Internal Error: Tried to execute a task on a nonexistent stream: %d", p_task->stream);
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	Ctrl_TaskQueue *p_host_kernel_queue = p_ctrl->pp_kernel_host_streams[p_task->stream];

	// wait for appropiate events from arguments according to their role
	for (int i = 0; i < p_task->n_arguments; i++) {
		if (p_task->p_roles[i] != KERNEL_INVAL) {
			HitTile        *p_tile           = (HitTile *)(p_task->pp_pointers[i]);
			Ctrl_Tile      *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
			Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
			Ctrl_Hip_Tile  *p_tile_data_hip  = p_tile_data_impl->tile.p_hip;
			KHitTile       *p_ktile          = (KHitTile *)((char *)p_task->p_arguments + p_task->p_displacements[i]);

			if (hit_tileIsNull(*p_tile)) {
				fprintf(stderr, "Warning: Launching task %s, skipping null tile on parameter %d (starting at 0)\n", p_task->p_func_name, i);
				fflush(stderr);
				continue;
			}

			if (p_tile_data_impl->device_status == CTRL_TILE_UNALLOC) {
				fprintf(stderr, "[Ctrl_Hip] Internal Error: Launching kernel %s with a tile with no device memory as parameter %d (starting at 0)\n", p_task->p_func_name, i);
				fflush(stderr);
				exit(EXIT_FAILURE);
			}

			// TODO @sergioalo care for other dimensions
			if (p_tile_data_hip->pitch != 0) {
				p_ktile->origAcumCard[1] = p_tile_data_hip->pitch / p_tile->baseExtent;
			}

			// if tile's role is IN or IO, is not updated on device and host has memory allocated transfer it
			if (p_task->p_roles[i] != KERNEL_OUT && p_tile_data_impl->device_status == CTRL_TILE_INVALID &&
				p_tile_data->host_status != CTRL_TILE_UNALLOC && p_ctrl->dependance_mode == CTRL_MODE_IMPLICIT) {
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
						fprintf(stderr, "[Ctrl_Hip] Warning: Tile with uninitialized data as input on kernel %s, parameter: %d (starting at 0)\n", p_task->p_func_name, i);
						fflush(stderr);
					}
				}
				Ctrl_Hip_EvalTaskMoveToInner(p_ctrl, p_tile);
			}

			if (p_tile_data_impl->device_status == CTRL_TILE_INVALID && p_task->p_roles[i] != KERNEL_OUT) {
				fprintf(stderr, "[Ctrl_Hip] Warning: Tile with uninitialized data as input on kernel %s, parameter: %d (starting at 0)\n", p_task->p_func_name, i);
				fflush(stderr);
			}

			// no need to wait for kw if last kw op was on the same stream
			if (p_tile_data_hip->streamid_last_kw != p_task->stream) {
				Ctrl_GenericEvent_StreamWait(p_tile_data_hip->host_last_kernel_write_event, p_host_kernel_queue);
				Ctrl_GenericEvent_StreamWait(p_tile_data_hip->dev_last_kernel_write_event, p_host_kernel_queue);
			}

			if (p_tile_data->host_status != CTRL_TILE_UNALLOC && !(p_tile_data_impl->device_status == CTRL_TILE_VALID && p_tile_data->host_status == CTRL_TILE_INVALID)) {
				Ctrl_GenericEvent_StreamWait(p_tile_data_hip->host_last_htd_event, p_host_kernel_queue);
				Ctrl_GenericEvent_StreamWait(p_tile_data_hip->dev_last_htd_event, p_host_kernel_queue);
			}

			if (p_task->p_roles[i] != KERNEL_IN) {
				// no need to wait for kw if last kw op was on the same stream
				if (p_tile_data_hip->streamid_last_kw != p_task->stream) {
					Ctrl_GenericEvent_StreamWait(p_tile_data_hip->host_last_kernel_read_event, p_host_kernel_queue);
					Ctrl_GenericEvent_StreamWait(p_tile_data_hip->dev_last_kernel_read_event, p_host_kernel_queue);
				}

				if (p_tile_data->host_status != CTRL_TILE_UNALLOC && !(p_tile_data_impl->device_status == CTRL_TILE_VALID && p_tile_data->host_status == CTRL_TILE_INVALID)) {
					Ctrl_GenericEvent_StreamWait(p_tile_data_hip->host_last_dth_event, p_host_kernel_queue);
					Ctrl_GenericEvent_StreamWait(p_tile_data_hip->dev_last_dth_event, p_host_kernel_queue);
				}
			}
		}
	}

	// wait for previous task to finish if policy is sync
	Ctrl_SyncWait(p_host_kernel_queue);

	// create request with info for kernel execution (stream is obtained in exec_task, no need to pass it here)
	Ctrl_Request request = {0};

	#ifdef _CTRL_HIPBLAS_
	request.hip.p_hipblas_handle = &(p_ctrl->hipblas_handle);
	#endif // _CTRL_HIPBLAS_

	Ctrl_GenericEvent kernel_event = Ctrl_GenericEvent_Create(CTRL_EVENT_TYPE_HIP, p_ctrl->global_id);

	p_task->request = request;
	p_task->event   = kernel_event;
	Ctrl_GenericEvent_Retain(p_task->event);
	Ctrl_TaskQueue_Push(p_host_kernel_queue, *p_task);

	// update events on arguments according to their roles
	for (int i = 0; i < p_task->n_arguments; i++) {
		if (p_task->p_roles[i] != KERNEL_INVAL) {
			HitTile        *p_tile           = (HitTile *)(p_task->pp_pointers[i]);
			Ctrl_Tile      *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
			Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
			Ctrl_Hip_Tile  *p_tile_data_hip  = p_tile_data_impl->tile.p_hip;

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
				Ctrl_CpuEvent_Record(&p_tile_data_hip->host_last_kernel_write_event.event.event_cpu, p_host_kernel_queue);
				Ctrl_GenericEvent_Release(p_tile_data_hip->dev_last_kernel_write_event);
				p_tile_data_hip->dev_last_kernel_write_event = kernel_event;
				Ctrl_GenericEvent_Retain(p_tile_data_hip->dev_last_kernel_write_event);
				p_tile_data_hip->streamid_last_kw = p_task->stream;
			}

			if (p_task->p_roles[i] != KERNEL_OUT) {
				Ctrl_CpuEvent_Record(&p_tile_data_hip->host_last_kernel_read_event.event.event_cpu, p_host_kernel_queue);
				Ctrl_GenericEvent_Release(p_tile_data_hip->dev_last_kernel_read_event);
				p_tile_data_hip->dev_last_kernel_read_event = kernel_event;
				Ctrl_GenericEvent_Retain(p_tile_data_hip->dev_last_kernel_read_event);
				p_tile_data_hip->streamid_last_kr = p_task->stream;
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

void Ctrl_Hip_HostTaskWait(Ctrl_Hip_Tile *p_tile, char rol, Ctrl_TaskQueue *p_queue) {
	Ctrl_GenericEvent_StreamWait(p_tile->host_last_dth_event, p_queue);
	Ctrl_GenericEvent_StreamWait(p_tile->dev_last_dth_event, p_queue);

	if (rol != KERNEL_IN) {
		Ctrl_GenericEvent_StreamWait(p_tile->host_last_htd_event, p_queue);
		Ctrl_GenericEvent_StreamWait(p_tile->dev_last_htd_event, p_queue);
	}
}

void Ctrl_Hip_SyncWait(Ctrl_Hip *p_ctrl, Ctrl_TaskQueue *p_queue) {
	Ctrl_GenericEvent_StreamWait(p_ctrl->host_seq_event, p_queue);
	Ctrl_GenericEvent_StreamWait(p_ctrl->dev_seq_event, p_queue);
}

void Ctrl_Hip_EvalTaskAllocTile(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task) {
	HitTile        *p_tile           = (HitTile *)(p_task->p_tile);
	Ctrl_Tile      *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];

	HIP_OP(hipSetDevice(p_ctrl->device));

	if (p_tile_data_impl->type == CTRL_TYPE_NULL)
		Ctrl_Hip_InitTile(p_ctrl, p_task);

	Ctrl_Hip_Tile *p_tile_data_hip = p_tile_data_impl->tile.p_hip;

	// TODO @sergioalo pinned for multiple devices?
	if (p_tile_data->host_status == CTRL_TILE_UNALLOC && (p_task->flags & CTRL_MEM_ALLOC_HOST || !(p_task->flags & CTRL_MEM_ALLOC_DEV))) {
		// Allocate host mem
		if ((p_task->flags & CTRL_MEM_PINNED) || (!(p_task->flags & CTRL_MEM_NOPINNED) && p_ctrl->default_alloc_mode == CTRL_MEM_PINNED)) {
			// Allocate host "pinned" memory
			p_tile_data->pinned = CTRL_TYPE_HIP;
			HIP_OP(hipHostMalloc(&(p_tile->data), ((size_t)(p_tile->origAcumCard[0])) * (p_tile->baseExtent), hipHostMallocPortable));
		} else {
			// Allocate host memory the usual way (on HIP this is probably never a good idea to do, but the option is there)
			p_tile_data->pinned = CTRL_TYPE_NULL;
			p_tile->data        = (void *)malloc((size_t)p_tile->acumCard * p_tile->baseExtent);
		}
		p_tile_data->host_status = CTRL_TILE_INVALID;
		p_tile->memPtr           = p_tile->data;
	}

	if (p_task->flags & CTRL_MEM_ALLOC_DEV || !(p_task->flags & CTRL_MEM_ALLOC_HOST)) {
		if (p_tile_data_impl->device_status != CTRL_TILE_UNALLOC) {
			fprintf(stderr, "[Ctrl_Hip] Warning: Device memory already allocated for this tile, ignoring this call.\n");
			fflush(stderr);
			return;
		}
		// Allocate device memory
		if (p_task->flags & CTRL_MEM_ALIGNED && hit_tileDims(*p_tile) == 2) {
			HIP_OP(hipMallocPitch(&(p_tile_data_hip->p_device_data), &p_tile_data_hip->pitch, (p_tile->baseExtent) * p_tile->card[1], p_tile->card[0]));
		} else {
			HIP_OP(hipMalloc(&(p_tile_data_hip->p_device_data), ((size_t)(p_tile->origAcumCard[0])) * (p_tile->baseExtent)));
		}

		p_tile_data_impl->device_status = CTRL_TILE_INVALID;
	}

	// TODO Habria que poner qstride, origAcumCard, y memStatus en p_tile_data con los dev (al tener dos espacios de memoria)
}

void Ctrl_Hip_EvalTaskSelectTile(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task) {
	HitTile *p_tile = p_task->p_tile;

	HIP_OP(hipSetDevice(p_ctrl->device));
	Ctrl_Hip_InitTile(p_ctrl, p_task);

	HitTile        *p_parent           = p_tile->ref;
	Ctrl_Tile      *p_tile_data        = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl *p_tile_data_impl   = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_Hip_Tile  *p_tile_data_hip    = p_tile_data_impl->tile.p_hip;
	Ctrl_Tile      *p_parent_data      = ((Ctrl_Tile *)(p_tile->ref->ext));
	Ctrl_Tile_Impl *p_parent_data_impl = &p_parent_data->p_impls[p_ctrl->global_id];
	Ctrl_Hip_Tile  *p_parent_data_hip  = p_parent_data_impl->tile.p_hip;

	if (p_tile->memStatus == HIT_MS_NOT_OWNER) {
		p_tile_data_impl->device_status = p_parent_data_impl->device_status;
		p_tile_data_hip->p_device_data  = (char *)p_parent_data_hip->p_device_data + ((char *)p_tile->data - (char *)p_parent->data);

		if (p_parent_data_hip->pitch != 0) {
			fprintf(stderr, "[Ctrl_Hip_EvalTaskSelectTile] Error: subselections of tiles with padding on the device (allocated with CTRL_MEM_ALIGNED) not supported.\n");
			exit(EXIT_FAILURE);
		}
	}
}

void Ctrl_Hip_EvalTaskFreeTile(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task) {
	HitTile        *p_tile           = p_task->p_tile;
	Ctrl_Tile      *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];
	Ctrl_Hip_Tile  *p_tile_data_hip  = p_tile_data_impl->tile.p_hip;

	// tile not initialized
	if (p_tile_data_impl->type == CTRL_TYPE_NULL) {
		fprintf(stderr, "[Ctrl_Hip] Warning: Free from tile not attached to ctrl.\n");
		return;
	}

	p_tile_data->valid_impls--;

	// Wait for all work related to this tile to finish
	Ctrl_Hip_WaitTileInner(p_ctrl, p_tile_data);

	// destroy events inside the tile
	Ctrl_GenericEvent_Release(p_tile_data_hip->host_last_kernel_read_event);
	Ctrl_GenericEvent_Release(p_tile_data_hip->host_last_kernel_write_event);
	Ctrl_GenericEvent_Release(p_tile_data_hip->host_last_dth_event);
	Ctrl_GenericEvent_Release(p_tile_data_hip->host_last_htd_event);
	Ctrl_GenericEvent_Release(p_tile_data_hip->dev_last_kernel_read_event);
	Ctrl_GenericEvent_Release(p_tile_data_hip->dev_last_kernel_write_event);
	Ctrl_GenericEvent_Release(p_tile_data_hip->dev_last_dth_event);
	Ctrl_GenericEvent_Release(p_tile_data_hip->dev_last_htd_event);

	// Remove tle from tile linked list
	if (p_tile_data_hip->p_tile_elem->p_prev != NULL) {
		p_tile_data_hip->p_tile_elem->p_prev->p_next = p_tile_data_hip->p_tile_elem->p_next;
	} else {
		p_ctrl->p_tile_list_head = p_tile_data_hip->p_tile_elem->p_next;
	}

	if (p_tile_data_hip->p_tile_elem->p_next != NULL) {
		p_tile_data_hip->p_tile_elem->p_next->p_prev = p_tile_data_hip->p_tile_elem->p_prev;
	} else {
		p_ctrl->p_tile_list_tail = p_tile_data_hip->p_tile_elem->p_prev;
	}

	// Clear node fields
	p_tile_data_hip->p_tile_elem->p_tile_ext = NULL;
	p_tile_data_hip->p_tile_elem->p_next     = NULL;
	p_tile_data_hip->p_tile_elem->p_prev     = NULL;

	// Free node
	free(p_tile_data_hip->p_tile_elem);

	if (p_tile->memStatus == HIT_MS_OWNER) {
		if (p_tile_data_impl->device_status != CTRL_TILE_UNALLOC) {
			// Free device image of the tile
			HIP_OP(hipFree(p_tile_data_hip->p_device_data));
			if (p_tile_data_hip->texture != 0) {
				hipDestroyTextureObject(p_tile_data_hip->texture);
			}
		}
	}

	// Clear tile fields
	p_tile_data_hip->p_ctrl = NULL;

	// Free tile
	free(p_tile_data_hip);

	// if this was the last ctrl the tile was attached to free the host stuff as well
	if (p_tile_data->valid_impls == 0) {
		Ctrl_FreeHostInner(p_tile);
	}
}

void Ctrl_Hip_EvalTaskMoveTo(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task) {
	HitTile        *p_tile           = p_task->p_tile;
	Ctrl_Tile      *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];

	if (hit_tileIsNull(*p_tile)) return;

	if (p_tile_data->host_status == CTRL_TILE_UNALLOC) {
		fprintf(stderr, "[Ctrl_Hip] Internal Error: Trying to move tile from host to device but host memory was not allocated\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	if (p_tile_data_impl->device_status == CTRL_TILE_UNALLOC) {
		fprintf(stderr, "[Ctrl_Hip] Internal Error: Trying to move tile from host to device but device memory was not allocated\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	if (p_tile_data->host_status == CTRL_TILE_INVALID) {
		fprintf(stderr, "[Ctrl_Hip] Warning: Moving a tile from host to device with invalid data on host memory\n");
		fflush(stderr);
	}

	// If tile is not updated perform the transfer
	if (!(p_tile_data->host_status == CTRL_TILE_VALID && p_tile_data_impl->device_status == CTRL_TILE_VALID) || p_ctrl->dependance_mode == CTRL_MODE_EXPLICIT) {
		Ctrl_Hip_EvalTaskMoveToInner(p_ctrl, p_tile);
	}
}

void Ctrl_Hip_EvalTaskMoveFrom(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task) {
	HitTile        *p_tile           = p_task->p_tile;
	Ctrl_Tile      *p_tile_data      = (Ctrl_Tile *)(p_tile->ext);
	Ctrl_Tile_Impl *p_tile_data_impl = &p_tile_data->p_impls[p_ctrl->global_id];

	if (hit_tileIsNull(*p_tile)) return;

	if (p_tile_data->host_status == CTRL_TILE_UNALLOC) {
		fprintf(stderr, "[Ctrl_Hip] Internal Error: Trying to move tile from device to host but host memory was not allocated\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	if (p_tile_data_impl->device_status == CTRL_TILE_UNALLOC) {
		fprintf(stderr, "[Ctrl_Hip] Internal Error: Trying to move tile from device to host but device memory was not allocated\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	if (p_tile_data_impl->device_status == CTRL_TILE_INVALID) {
		fprintf(stderr, "[Ctrl_Hip] Warning: Moving a tile from device to host with invalid data on device memory\n");
		fflush(stderr);
	}

	// If tile is not updated perform the transfer
	if (!(p_tile_data->host_status == CTRL_TILE_VALID && p_tile_data_impl->device_status == CTRL_TILE_VALID) || p_ctrl->dependance_mode == CTRL_MODE_EXPLICIT) {
		Ctrl_Hip_EvalTaskMoveFromInner(p_ctrl, p_tile);
	}
}

void Ctrl_Hip_EvalTaskWaitTile(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task) {
	HitTile *p_tile = p_task->p_tile;

	if (hit_tileIsNull(*p_tile)) return;

	Ctrl_Hip_WaitTileInner(p_ctrl, (Ctrl_Tile *)(p_tile->ext));
}

void Ctrl_Hip_EvalTaskSetDependanceMode(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task) {
	p_ctrl->dependance_mode = p_task->flags;
}
///@endcond
