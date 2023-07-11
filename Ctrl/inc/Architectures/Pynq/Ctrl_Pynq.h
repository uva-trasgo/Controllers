#ifndef _CTRL_PYNQ_H_
#define _CTRL_PYNQ_H_
///@cond INTERNAL
/**
 * @file Ctrl_PYNQ.h
 * @author Trasgo Group
 * @brief Ctrl implementation for cpu devices.
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

#include <omp.h>
#ifndef _CTRL_COMPILE_PYNQ_KERNEL
#include <hwloc.h>
#endif // _CTRL_COMPILER_PYNQ_KERNEL

#include <stdbool.h>
#include <stdio.h>
#include "hitmap2.h"

#include "Core/Ctrl_Policy.h"
#include "Core/Ctrl_Type.h"
#include "Core/Ctrl_TaskQueue.h"
#include "Core/Ctrl_Tile.h"
#include "Core/Ctrl_KHitTile.h"
#include "Core/Ctrl_Request.h"

#include "Kernel/Ctrl_KernelProto.h"
#include "Kernel/Ctrl_ImplType.h"

#include "Architectures/Pynq/Ctrl_Pynq_Tile.h"

// TODO: this parameter should be set by the user
#define N_RR 2

extern int n_launched_kernels;
//pthread_mutex_t lock_swap;
omp_lock_t lock_swap;
// This lock is used to synchronise swap operations with the execution of kernels
omp_lock_t lock_RR[N_RR];
omp_lock_t lock_wait;
extern int launched_kernel[N_RR];
extern int swap_depends_on_launch[N_RR];
extern int current_launch_task_id;
extern int current_host_task_id;
extern int evicted_host_task_id;
extern int host_task_ready;
extern int currently_swapping;

/**
 * Launch a kernel to the ctrl queue
 * @hideinitializer
 * 
 * @param p_ctrl pointer to the ctrl to launch the kernel.
 * @param name name of the kernel to be launched.
 * @param threads thread block to launch the kernel with. (Ctrl_Thread).
 * @param group block sizes for this kernel execution.
 * @param rr reconfigurable of the kernel to be launched.
 * 		Optional, if a block with 0 dimensions is passed (such as CTRL_THREAD_NULL), default characterization is used instead.
 * @param ... arguments passed to the kernel.
 * 
 * @see Ctrl_Launch, Ctrl_Thread
 */
#define CTRL_PYNQ_LAUNCH( p_ctrl, name, threads, group, _rr, ... ) \
	case CTRL_TYPE_PYNQ: \
		if(group.dims == 0){ \
			Ctrl_Task task =  Ctrl_KernelTaskCreate_##name( CTRL_TYPE_PYNQ, threads, blocksize_PYNQ_##name, 0, CTRL_KERNEL_ARGS_TO_POINTERS( __VA_ARGS__ ) ); \
            task.rr = _rr; \
			Ctrl_LaunchKernel(p_ctrl, task); \
		}else{ \
			Ctrl_Task task = Ctrl_KernelTaskCreate_##name( CTRL_TYPE_PYNQ, threads, group, 0, CTRL_KERNEL_ARGS_TO_POINTERS( __VA_ARGS__ ) ); \
            task.rr = _rr; \
			Ctrl_LaunchKernel(p_ctrl, task); \
		} \
		break;

/**
 * Launch a kernel to a specific stream of the ctrl queue. On PYNQ Ctrls \p stream is ignored as there is only one kernel queue.
 * @hideinitializer
 * 
 * @param p_ctrl: pointer to the ctrl to launch the kernel
 * @param name: name of the kernel to be launched
 * @param threads: thread block to launch the kernel with. (Ctrl_Thread)
 * @param group block sizes for this kernel execution.
 *      Optional, if a block with 0 dimensions is passed (such as CTRL_THREAD_NULL), default characterization is used instead.
 * @param stream: stream to launch the kernel to. Ignored on PYNQ.
 * @param ...: arguments passed to the kernel.
 * 
 * @see Ctrl_LaunchToStream, Ctrl_Thread
 */
#define CTRL_PYNQ_LAUNCH_STREAM( p_ctrl, name, threads, group, stream, ... ) CTRL_PYNQ_LAUNCH(p_ctrl, name, threads, group, __VA_ARGS__)

/**
 * PYNQ implementation of abstract ctrl
 */
typedef struct Ctrl_PYNQ{
	int								n_cores;			/**< Number of threads to be used when executing kernels*/

	bool							mem_moves;			/**< Flag to determine if memory transfers between host ad device should be made or not */

	struct Ctrl_PYNQ_Tile_List		*p_tile_list_head;	/**< Head of the list of tiles associate to this ctrl */
	struct Ctrl_PYNQ_Tile_List		*p_tile_list_tail;	/**< Tail of the list of tiles associate to this ctrl */

	#ifdef _CTRL_QUEUE_
	omp_lock_t						*p_lock_first_host;	/**< Lock used for sync between main thread and queue manager thread */
	omp_lock_t						*p_lock_first_ctrl;	/**< Lock used for sync between main thread and queue manager thread */
	omp_lock_t						*p_lock_host;		/**< Lock used for sync between main thread and queue manager thread */
	omp_lock_t						*p_lock_ctrl;		/**< Lock used for sync between main thread and queue manager thread */
	#endif //_CTRL_QUEUE_

	Ctrl_Policy						policy;				/**< Policy to be used by this ctrl (sync or async) */

	Ctrl_TaskQueue					*p_kernel_stream[N_RR];	/**< PYNQ stream for kernel execution */
	Ctrl_TaskQueue					*p_moveTo_stream;	/**< PYNQ stream for host to device memory transfers */
	Ctrl_TaskQueue					*p_moveFrom_stream;	/**< PYNQ stream for host to device memory transfers */
	Ctrl_PYNQEvent					event_seq;			/**< Event used for sync policy */

	int								dependance_mode;	/**< Dependance mode to be used by this ctrl */
} Ctrl_PYNQ;


#ifdef __cplusplus
	extern "C" {
#endif
/**
 * Create the controller and its corresponding variables.
 * 
 * @param p_ctrl Controller to be created.
 * @param policy Policy to be used by the contrller.
 * @param n_cores Number of cores to be used.
 * @param p_numa_nodes Pointer to the list of NUMA nodes to be used for kernel affinity.
 * @param n_numa_nodes Number of NUMA nodes passed in \p p_numa_nodes.
 * @param mem_moves Flag to indicate if copies between device and host should be made or accesses should be zero copies.
 * 
 * @pre if \p p_numa_nodes is NULL then \p n_numa_nodes must be 0
 */
void Ctrl_PYNQ_Create(Ctrl_PYNQ *p_ctrl, Ctrl_Policy policy, int device);

/**
 * Evaluate a task on a PYNQ ctrl.
 * 
 * @param p_ctrl ctrl to execute the task.
 * @param p_task pointer to the task to be evaluated.
 */
void Ctrl_PYNQ_EvalTask(Ctrl_PYNQ *p_ctrl, Ctrl_Task *p_task);

/**
 * Get the number of extra threads needed by \p p_ctrl.
 * 
 * @param p_ctrl Ctrl we want to get the number of extra threads of.
 * 
 * @returns the number of extra threads needed by \p p_ctrl.
 */
int Ctrl_PYNQ_GetNumThreads(Ctrl_PYNQ *p_ctrl);

/**
 * Set affinity of threads used by \p p_ctrl and send them to therir tasks.
 * @param p_ctrl pointer to the ctrl.
 * @param topo topology of the computer.
 * @param node index of NUMA node used as host.
 */
//void Ctrl_PYNQ_ThreadInit(Ctrl_PYNQ *p_ctrl);
#ifndef _CTRL_COMPILE_PYNQ_KERNEL
void Ctrl_PYNQ_ThreadInit(Ctrl_PYNQ *p_ctrl, hwloc_topology_t topo, int node);
#endif // _CTRL_COMPILE_PYNQ_KERNEL

void Ctrl_KernelWrapper_Swap(int swap_id, int rr, char * bitstream_path);
void Ctrl_KernelWrapper_KernelSync();
#ifdef __cplusplus
	}
#endif
///@endcond

extern int enable;
extern int disable;

#ifdef _EXP_MODE
extern FILE * exp_file;
extern double init_time;
#endif
#endif /* _CTRL_PYNQ_H_ */
