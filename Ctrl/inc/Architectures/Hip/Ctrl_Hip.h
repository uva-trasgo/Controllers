#ifndef _CTRL_HIP_H_
#define _CTRL_HIP_H_
///@cond INTERNAL
/**
 * @file Ctrl_Hip.h
 * @brief Ctrl implementation for HIP devices.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include <omp.h>
#include <stdbool.h>

#include <hip/hip_runtime.h>

#ifdef _CTRL_HIPBLAS_
#include <hipblas/hipblas.h>
#endif // _CTRL_HIPBLAS_

#include "hitmap2.h"

#include "Core/Ctrl_Info.h"
#include "Core/Ctrl_KHitTile.h"
#include "Core/Ctrl_Policy.h"
#include "Core/Ctrl_Request.h"
#include "Core/Ctrl_TaskQueue.h"
#include "Core/Ctrl_TexDesc.h"
#include "Core/Ctrl_Tile.h"
#include "Core/Ctrl_Type.h"
#include "Core/Ctrl_Config.h"

#include "Kernel/Ctrl_ImplType.h"
#include "Kernel/Ctrl_KernelProto.h"

#include "Architectures/Hip/Ctrl_Hip_Helper.h"
#include "Architectures/Hip/Ctrl_Hip_Request.h"

/**
 * Launch a kernel to a specific stream of the ctrl queue
 * @hideinitializer
 *
 * @param p_ctrl: pointer to the ctrl to launch the kernel
 * @param name: name of the kernel to be launched
 * @param threads: thread block to launch the kernel with. (Ctrl_Thread)
 * @param group block sizes for this kernel execution.
 *      Optional, if a block with 0 dimensions is passed (such as CTRL_THREAD_NULL), default characterization is used instead.
 * @param stream: stream to launch the kernel to.
 * @param ...: arguments passed to the kernel.
 *
 * @see Ctrl_LaunchToStream, Ctrl_Thread
 */
#define CTRL_HIP_LAUNCH_STREAM(p_ctrl, name, threads, group, stream, ...)                                                                                                                                 \
	case CTRL_TYPE_HIP:                                                                                                                                                                                   \
		if (group.dims == 0) {                                                                                                                                                                            \
			Ctrl_LaunchKernel(p_ctrl, Ctrl_KernelTaskCreate_##name(p_ctrl, threads, CTRL_KERNEL_HIP_CHAR_threads(name, CTRL_KERNEL_HIP_ARCH_KEPLER), stream, CTRL_KERNEL_ARGS_TO_POINTERS(__VA_ARGS__))); \
		} else {                                                                                                                                                                                          \
			Ctrl_LaunchKernel(p_ctrl, Ctrl_KernelTaskCreate_##name(p_ctrl, threads, group, stream, CTRL_KERNEL_ARGS_TO_POINTERS(__VA_ARGS__)));                                                           \
		}                                                                                                                                                                                                 \
		break;

/**
 * HIP implementation of abstract ctrl
 */
typedef struct Ctrl_Hip {
	int                    global_id;        /**< Id of this ctrl with respect to other ctrls */
	int                    device;           /**< Index of the HIP gpu device used by the ctrl */
	Ctrl_GenericEvent      host_seq_event;   /**< Host event used for sync policy */
	Ctrl_GenericEvent      dev_seq_event;    /**< Device event used for sync policy */
	struct Ctrl_Tile_List *p_tile_list_head; /**< Head of the list of tiles associated to this ctrl */
	struct Ctrl_Tile_List *p_tile_list_tail; /**< Tail of the list of tiles associated to this ctrl */

	#ifdef _CTRL_HIPBLAS_
	hipblasHandle_t hipblas_handle; /**< Handle for hipblas lib operations */
	#endif // _CTRL_HIPBLAS_

	Ctrl_Policy      policy;                  /**< Policy to be used by this ctrl (sync or async) */
	int              dependance_mode;         /**< Dependance mode to be used by this ctrl */
	int              n_kernel_streams;        /**< Number of HIP streams for kernel launching available to this ctrl */
	int              default_alloc_mode;      /**< Default allocation mode. On HIP it will be pinned */
	hipStream_t     *p_kernel_driver_streams; /**< HIP streams for kernels */
	Ctrl_TaskQueue **pp_kernel_host_streams;  /**< Host queues for kernels */
	hipStream_t      htd_driver_stream;       /**< HIP stream for HTD memory transfers */
	Ctrl_TaskQueue  *p_htd_host_stream;       /**< Host queue for HTD memory transfers */
	hipStream_t      dth_driver_stream;       /**< HIP stream for DTH memory transfers */
	Ctrl_TaskQueue  *p_dth_host_stream;       /**< Host queue for DTH memory transfers */
	int              alignment;               /**< Alignment in bytes to use when allocating aligned tiles on this device */
} Ctrl_Hip;

/**
 * Create the controller and its corresponding variables.
 *
 * @param p_ctrl Controller to be created.
 * @param policy Policy for this ctrl to be used.
 * @param dev Configuration for this ctrl. @see DEVICE_SELECTION.md for more information.
 */
void Ctrl_Hip_Create(Ctrl_Hip *p_ctrl, Ctrl_Policy policy, Ctrl_Config_Dev dev);

/**
 * Evaluate a task on a HIP ctrl.
 *
 * @param p_ctrl Ctrl to execute the task.
 * @param p_task Pointer to the task to be evaluated.
 */
void Ctrl_Hip_EvalTask(Ctrl_Hip *p_ctrl, Ctrl_Task *p_task);

/**
 * @brief Execute a hip task.
 *
 * Enqueue a ready to execute operation to the appropiate HIP stream
 *
 * @param p_task Task to execute
 * @param p_ctrl Ctrl responsible for the task
 */
void Ctrl_Hip_ExecTask(Ctrl_Task *p_task, Ctrl_Hip *p_ctrl);

/**
 * Get the number of host queues used by \p p_ctrl.
 *
 * @param p_ctrl Ctrl to get the number of queues from
 * @return number of host queues used by \p p_ctrl
 */
int Ctrl_Hip_GetNumQueues(Ctrl_Hip *p_ctrl);

/**
 * Get the pointers to the host queues used by \p p_ctrl on list \p pp_queues
 *
 * @param p_ctrl Ctrl to get the queues from.
 * @param pp_queues [out] Pointer to pointers to the host queues used by this ctrl
 * @return Pointer right after the queue pointers stored on \p pp_queues
 *
 * @pre \p pp_queues must have enough memory allocated to store all queues from this ctrl.
 * @see Ctrl_Hip_GetNumQueues
 */
Ctrl_TaskQueue **Ctrl_Hip_GetHostQueues(Ctrl_Hip *p_ctrl, Ctrl_TaskQueue **pp_queues);

/**
 * Enqueue appropiate wait operations for a tile in a host task.
 *
 * @param p_tile tile of the host task
 * @param rol rol of \p p_tile
 * @param p_queue queue to send the wait to.
 */
void Ctrl_Hip_HostTaskWait(Ctrl_Hip_Tile *p_tile, char rol, Ctrl_TaskQueue *p_queue);

/**
 * Enqueue wait for seq event on \p p_queue
 *
 * @param p_ctrl ctrl containing the event.
 * @param p_queue queue to send the wait to.
 */
void Ctrl_Hip_SyncWait(Ctrl_Hip *p_ctrl, Ctrl_TaskQueue *p_queue);

/**
 * Enqueue wait for appropiate events from \p p_tile_data for a MoveTo operation in \p p_queue.
 *
 * @param p_tile_data metadata of tile to wait for
 * @param p_queue queue to enqueue the events on
 */
void Ctrl_Hip_MoveToWait(Ctrl_Hip_Tile *p_tile_data, Ctrl_TaskQueue *p_queue);

/**
 * Enqueue wait for appropiate events from \p p_tile_data for a MoveFrom operation in \p p_queue.
 *
 * @param p_tile_data metadata of tile to wait for
 * @param p_queue queue to enqueue the events on
 */
void Ctrl_Hip_MoveFromWait(Ctrl_Hip_Tile *p_tile_data, Ctrl_TaskQueue *p_queue);

/**
 * Enqueue memory transfer from device to host task on host queue.
 *
 * @param p_ctrl Pointer to the ctrl attached to the tile to be moved.
 * @param p_tile Pointer to the tile to be moved.
 *
 * @see Ctrl_Hip_EvalTaskMoveFrom
 */
void Ctrl_Hip_EvalTaskMoveFromInner(Ctrl_Hip *p_ctrl, HitTile *p_tile);

/**
 * Get information of the device asociated with \p p_ctrl.
 * @param p_ctrl ctrl to get the info from.
 * @param p_info struct to store the info into.
 */
void Ctrl_Hip_GetInfo(Ctrl_Hip *p_ctrl, Ctrl_Info *p_info);

/**
 * Create a new texture object asociated with \p p_tile and \p p_ctrl
 *
 * @param p_ctrl
 * @param p_tile
 * @param tex_desc Confguration for the texture object
 *
 * @pre \p p_tile must have device memory allocated with \p p_ctrl with a suitable alignment
 * @see Ctrl_Alloc
 */
void Ctrl_Hip_CreateTex(Ctrl_Hip *p_ctrl, HitTile *p_tile, Ctrl_TexDesc tex_desc);

/**
 * Get the device ptr of tile \p p_tile on ctrl \p p_ctrl.
 *
 * If \p tile is not attached to \p ctrl or has no device memory allocated NULL is returned.
 *
 * @param p_ctrl pointer to ctrl.
 * @param p_tile tile to get de device ptr from.
 * @return pointer to device memory for \p tile on device \p ctrl
 */
void *Ctrl_Hip_GetDevPtr(Ctrl_Hip *p_ctrl, HitTile *p_tile);

/**
 * @brief Return the duration of the last kernel or memory transfer operation performed over \p tile.
 *
 * The last operation enqueued for \p tile must be completed before calling this function.
 *
 * @param p_ctrl Ctrl \p p_tile is associated to.
 * @param p_tile HitTile attached to \p p_ctrl.
 *
 * @return Duration of the last op over \p p_tile in seconds.
 */
double Ctrl_Hip_TimeLastOp(Ctrl_Hip *p_ctrl, HitTile *p_tile);

/**
 * @brief Try to allocate pinned memory for \p p_tile
 *
 * This function may not actually allocate memory depending on flags and \p p_ctrl.
 * Use the return value to check this.
 *
 * @param p_ctrl Ctrl to allocate memory
 * @param p_tile Tile to allocate memory for
 * @param flags Ctrl tile allocation flags
 * @return Wether memory was allocated or not
 */
bool Ctrl_Hip_AllocPinned(Ctrl_Hip *p_ctrl, HitTile *p_tile, int flags);
///@endcond
#endif /* _CTRL_HIP_H_ */
