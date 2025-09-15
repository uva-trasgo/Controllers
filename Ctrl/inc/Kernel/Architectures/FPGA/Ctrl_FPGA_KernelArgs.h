#ifndef _CTRL_KERNEL_FPGA_ARGS_H_
#define _CTRL_KERNEL_FPGA_ARGS_H_
///@cond INTERNAL
/**
 * @file Ctrl_FPGA_KernelArgs.h
 * @brief Macros to cast Ctrl FPGA tiles to KHitTiles for their use in kernels.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#ifndef CTRL_FPGA_KERNEL_FILE
#include "Architectures/FPGA/Ctrl_FPGA_Tile.h"
#include "Kernel/Ctrl_ImplType.h"

/**
 * Macro used to convert tiles into \e KHitTiles for kernel execution.
 * @hideinitializer
 *
 * @param name name of the tile parameter.
 *
 * @see KHitTile
 */
#define CTRL_KERNEL_FPGA_KTILE_DEVICE_DATA(name)                                                                     \
	case CTRL_TYPE_FPGA: {                                                                                           \
		Ctrl_FPGA_Tile *p_tile_data = (Ctrl_FPGA_Tile *)(((Ctrl_Tile *)name->ext)->p_impls[p_ctrl->id].tile.p_fpga); \
		k_##name##_void.data        = (void *)(&(p_tile_data->device_data));                                         \
		break;                                                                                                       \
	}

#else // !CTRL_FPGA_KERNEL_FILE

#define Ctrl_Init_KTile(type, name)                                         \
	KHitTile_##type name;                                                   \
	name.data                                             = name##_data;    \
	*((fpga_wrapper_KHitTile_##type *)&name.origAcumCard) = name##_wrapper; \
	name.data += name.offset;

#define Ctrl_Init_KTileR(type, name)                                        \
	KHitTileR_##type name;                                                  \
	name.data                                             = name##_data;    \
	*((fpga_wrapper_KHitTile_##type *)&name.origAcumCard) = name##_wrapper; \
	name.data += name.offset;

#ifdef DOXYGEN
/**
 * In the FPGA backend, KHitTiles are unfolded into a KHitTile wrapper and a data pointer
 * (structures with pointers cannot be passed as kernel arguments in OpenCL).
 * Using this macro when describing KHitTile kernel arguments replaces the need
 * for the old, FPGA-backend-exclusvie, PARAMS(<arg roles>) macro.
 * @hideinitializer
 *
 * @param type The base type of the KHitTile
 * @param name The name of the KHitTile variable
 *
 * @see KHitTileR_arg
 */
#define KHitTile_arg(type, name)
#else // !DOXYGEN
#define header_KHitTile_arg(type, name) , fpga_wrapper_KHitTile_##type name##_wrapper, data_KHitTile_##type name##_data
#define init_KHitTile_arg(type, name)   Ctrl_Init_KTile(type, name)
#endif // DOXYGEN

#ifdef DOXYGEN
/**
 * This is used for Restricted KHitTiles, which are KHitTiles which point to restricted data
 * (see the restrict C keyword).
 *
 * In the FPGA backend, KHitTiles are unfolded into a KHitTile wrapper and a data pointer
 * (structures with pointers cannot be passed as kernel arguments in OpenCL).
 * Using this macro when describing KHitTile kernel arguments replaces the need
 * for the old, FPGA-backend-exclusvie, PARAMS(<arg roles>) macro.
 * @hideinitializer
 *
 * @param type The base type of the KHitTileR
 * @param name The name of the KHitTile variable
 *
 * @see KHitTile_arg
 */
#define KHitTileR_arg(type, name)
#else // !DOXYGEN
#define header_KHitTileR_arg(type, name) , fpga_wrapper_KHitTile_##type name##_wrapper, data_KHitTileR_##type name##_data
#define init_KHitTileR_arg(type, name)   Ctrl_Init_KTileR(type, name)
#endif // DOXYGEN

#ifdef DOXYGEN
/**
 * This macro is used to unify argument declaration syntax in the FPGA backend.
 * @hideinitializer
 *
 * @see KHitTile_arg, KHitTileR_arg
 */
#define K_arg(type, name)
#else // !DOXYGEN
#define header_K_arg(type, name) , type name
#define init_K_arg(type, name)
#endif // DOXYGEN

#define CTRL_KERNEL_FPGA_PARSE_ARGS(...)               CTRL_KERNEL_FPGA_PARSE_ARGS_N(CTRL_COUNTPARAM(__VA_ARGS__), __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PARSE_ARGS_N(n_args, ...)     CTRL_KERNEL_FPGA_PARSE_ARGS_N_EXP(n_args, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PARSE_ARGS_N_EXP(n_args, ...) CTRL_KERNEL_FPGA_PROCESS_ARGS_##n_args(header, __VA_ARGS__)

#define CTRL_KERNEL_FPGA_INIT_ARGS(...)               CTRL_KERNEL_FPGA_INIT_ARGS_N(CTRL_COUNTPARAM(__VA_ARGS__), __VA_ARGS__)
#define CTRL_KERNEL_FPGA_INIT_ARGS_N(n_args, ...)     CTRL_KERNEL_FPGA_INIT_ARGS_N_EXP(n_args, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_INIT_ARGS_N_EXP(n_args, ...) CTRL_KERNEL_FPGA_PROCESS_ARGS_##n_args(init, __VA_ARGS__)

#define CTRL_KERNEL_FPGA_PROCESS_ARGS_1(processing_type, arg)       processing_type##_##arg
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_2(processing_type, arg, ...)  processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_1(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_3(processing_type, arg, ...)  processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_2(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_4(processing_type, arg, ...)  processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_3(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_5(processing_type, arg, ...)  processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_4(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_6(processing_type, arg, ...)  processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_5(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_7(processing_type, arg, ...)  processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_6(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_8(processing_type, arg, ...)  processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_7(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_9(processing_type, arg, ...)  processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_8(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_10(processing_type, arg, ...) processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_9(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_11(processing_type, arg, ...) processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_10(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_12(processing_type, arg, ...) processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_11(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_13(processing_type, arg, ...) processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_12(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_14(processing_type, arg, ...) processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_13(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_15(processing_type, arg, ...) processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_14(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_16(processing_type, arg, ...) processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_15(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_17(processing_type, arg, ...) processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_16(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_18(processing_type, arg, ...) processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_17(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_19(processing_type, arg, ...) processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_18(processing_type, __VA_ARGS__)
#define CTRL_KERNEL_FPGA_PROCESS_ARGS_20(processing_type, arg, ...) processing_type##_##arg CTRL_KERNEL_FPGA_PROCESS_ARGS_19(processing_type, __VA_ARGS__)

#endif // CTRL_FPGA_KERNEL_FILE

///@endcond
#endif // _CTRL_KERNEL_FPGA_ARGS_H_
