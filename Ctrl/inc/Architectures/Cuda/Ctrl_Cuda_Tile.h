#ifndef _CTRL_CUDA_TILE_H_
#define _CTRL_CUDA_TILE_H_
///@cond INTERNAL
/**
 * @file Ctrl_Cuda_Tile.h
 * @brief Ctrl tile implentation for CUDA devices.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include <cuda_runtime_api.h>
#include <stdbool.h>

#include "Core/Ctrl_TaskQueue.h"

typedef struct Ctrl_Cuda Ctrl_Cuda;

/**
 * \brief Metadata from a tile associated to a CUDA ctrl
 */
typedef struct Ctrl_Cuda_Tile {
	struct Ctrl_Cuda      *p_ctrl;                       /**< Pointer to the ctrl to which this tile is associated */
	struct Ctrl_Tile_List *p_tile_elem;                  /**< Node of the linked list containing all tiles of the ctrl to which this tile is associated */
	void                  *p_device_data;                /**< Pointer to device image of this tile */
	cudaTextureObject_t    texture;                      /**< For texture memory allocations. */
	Ctrl_GenericEvent      host_last_kernel_read_event;  /**< Host event to sincronyze tasks, represents last write operation on device*/
	Ctrl_GenericEvent      host_last_kernel_write_event; /**< Host event to sincronyze tasks, represents last read operation on device*/
	Ctrl_GenericEvent      host_last_dth_event;          /**< Host event to sincronyze tasks, represents last DTH comunication*/
	Ctrl_GenericEvent      host_last_htd_event;          /**< Host event to sincronyze tasks, represents last HTD comunication*/
	Ctrl_GenericEvent      dev_last_kernel_read_event;   /**< Driver event to sincronyze tasks, represents last write operation on device*/
	Ctrl_GenericEvent      dev_last_kernel_write_event;  /**< Driver event to sincronyze tasks, represents last read operation on device*/
	Ctrl_GenericEvent      dev_last_dth_event;           /**< Driver event to sincronyze tasks, represents last DTH comunication*/
	Ctrl_GenericEvent      dev_last_htd_event;           /**< Driver event to sincronyze tasks, represents last HTD comunication*/
	Ctrl_GenericEvent      last_op_start;                /**< Driver event to time tasks, represents last op. */
	Ctrl_GenericEvent      last_op_stop;                 /**< Driver event to time tasks, represents last op. */
	int                    streamid_last_kw;             /**< Id of the stream of the last kernel write operation on this tile. For event optimizations */
	int                    streamid_last_kr;             /**< Id of the stream of the last kernel read operation on this tile. For event optimizations */
} Ctrl_Cuda_Tile;

///@endcond
#endif //_CTRL_CUDA_TILE_H_
