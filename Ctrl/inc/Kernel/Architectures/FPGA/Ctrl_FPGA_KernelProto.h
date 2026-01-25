#ifndef _CTRL_FPGA_KERNELPROTO_H_
#define _CTRL_FPGA_KERNELPROTO_H_
///@cond INTERNAL
/**
 * @file Ctrl_FPGA_KernelProto.h
 * @brief Macros to generate the code and manage FPGA kernels.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

// @sergioalo formatter does not respect ifdef indentation so leaving it off in this file

#ifndef CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#endif // CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include "Kernel/Ctrl_KernelArgs.h"
#include "Kernel/Ctrl_Thread.h"

#ifndef _CTRL_FPGA_KERNEL_FILE_
#include "Core/Ctrl_Request.h"
#endif // _CTRL_FPGA_KERNEL_FILE_

#define __CTRL_FPGA_KERNEL_OPTIMIZE_FOR(simd_lanes, sx, sy, sz) \
	__attribute__((reqd_work_group_size(sx, sy, sz)))           \
	__attribute__((num_simd_work_items(simd_lanes))) // Might not work, depending on aoc version used

#define __CTRL_FPGA_KERNEL_REPLICATE(...) __attribute__((num_compute_units(__VA_ARGS__)))

#define CTRL_KERNEL_FN_FPGA(name, type, subtype, ...) \
	CTRL_KERNEL_FN_FPGA_##subtype(name, type, subtype, __VA_ARGS__)

#define CTRL_KERNEL_FN_FPGA_TASK(name, type, subtype, ...)                                                            \
	__attribute__((max_global_work_dim(0)))                                                                           \
	__attribute__((uses_global_work_offset(0)))                                                                       \
	__kernel void                                                                                                     \
	ctrl_kernel_fpga_##type##_##subtype##_##name(Ctrl_Thread ctrl_threads CTRL_KERNEL_FPGA_PARSE_ARGS(__VA_ARGS__)) { \
		CTRL_KERNEL_FPGA_INIT_ARGS(__VA_ARGS__)

#define CTRL_KERNEL_FN_FPGA_NDRANGE(name, type, subtype, ...)                                                         \
	__attribute__((uses_global_work_offset(0)))                                                                       \
	__kernel void                                                                                                     \
	ctrl_kernel_fpga_##type##_##subtype##_##name(Ctrl_Thread ctrl_threads CTRL_KERNEL_FPGA_PARSE_ARGS(__VA_ARGS__)) { \
		CTRL_KERNEL_FPGA_INIT_ARGS(__VA_ARGS__)                                                                       \
		int thr_i = 0;                                                                                                \
		int thr_j = 0;                                                                                                \
		int thr_k = 0;                                                                                                \
		if (ctrl_threads.dims == 3) {                                                                                 \
			thr_k = get_global_id(0);                                                                                 \
			thr_j = get_global_id(1);                                                                                 \
			thr_i = get_global_id(2);                                                                                 \
		} else if (ctrl_threads.dims == 2) {                                                                          \
			thr_j = get_global_id(0);                                                                                 \
			thr_i = get_global_id(1);                                                                                 \
		} else {                                                                                                      \
			thr_i = get_global_id(0);                                                                                 \
		}                                                                                                             \
		if (thr_i >= (int)ctrl_threads.i || thr_j >= (int)ctrl_threads.j || thr_k >= (int)ctrl_threads.k)             \
			return;
/*
 * @brief: CTRL_PARSE_ARGS generates the arguments passed to the FPGA kernels.
 * Recall they must be passed by value in the case of INVAL while.
 *
 * IN and OUT must be split in a wrapper and a pointer to data. The wrapper will be of the form KHitTile_wrapper <name>_wrapper.
 *
 * This macro is leveraged to have both the type of the wrapper and the index of the displacement at the moment of the creation of the arguments.
 */
#define CTRL_PARSE_ARGS(kernel, n_args, ...) CTRL_PARSE_ARGS_##n_args(kernel, 0, __VA_ARGS__)

#define CTRL_PARSE_ARGS_1(kernel, arg_idx, role, type, name, ...)  CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name)
#define CTRL_PARSE_ARGS_2(kernel, arg_idx, role, type, name, ...)  CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_1(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_3(kernel, arg_idx, role, type, name, ...)  CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_2(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_4(kernel, arg_idx, role, type, name, ...)  CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_3(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_5(kernel, arg_idx, role, type, name, ...)  CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_4(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_6(kernel, arg_idx, role, type, name, ...)  CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_5(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_7(kernel, arg_idx, role, type, name, ...)  CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_6(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_8(kernel, arg_idx, role, type, name, ...)  CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_7(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_9(kernel, arg_idx, role, type, name, ...)  CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_8(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_10(kernel, arg_idx, role, type, name, ...) CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_9(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_11(kernel, arg_idx, role, type, name, ...) CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_10(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_12(kernel, arg_idx, role, type, name, ...) CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_11(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_13(kernel, arg_idx, role, type, name, ...) CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_12(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_14(kernel, arg_idx, role, type, name, ...) CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_13(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_15(kernel, arg_idx, role, type, name, ...) CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_14(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_16(kernel, arg_idx, role, type, name, ...) CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_15(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_17(kernel, arg_idx, role, type, name, ...) CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_16(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_18(kernel, arg_idx, role, type, name, ...) CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_17(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_19(kernel, arg_idx, role, type, name, ...) CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_18(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_20(kernel, arg_idx, role, type, name, ...) CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_19(kernel, arg_idx + 1, __VA_ARGS__)
#define CTRL_PARSE_ARGS_21(kernel, arg_idx, role, type, name, ...) CTRL_PARSE_ARGS_##role(kernel, arg_idx, type, name) CTRL_PARSE_ARGS_20(kernel, arg_idx + 1, __VA_ARGS__)

#define CTRL_PARSE_ARGS_IN(kernel, arg_idx, type, name)                                                                                     \
	KHitTile             *p_ktile_##type##_##subtype##_##name = (KHitTile *)((uint8_t *)args_list + request.fpga.p_displacements[arg_idx]); \
	KHitTile_fpga_wrapper ktile_fpga_wrapper_##type##_##subtype##_##name;                                                                   \
	for (int i = 0; i < HIT_MAXDIMS + 1; i++)                                                                                               \
		ktile_fpga_wrapper_##type##_##subtype##_##name.origAcumCard[i] = p_ktile_##type##_##subtype##_##name->origAcumCard[i];              \
	for (int i = 0; i < HIT_MAXDIMS; i++)                                                                                                   \
		ktile_fpga_wrapper_##type##_##subtype##_##name.card[i] = hit_tileDimCard((*p_ktile_##type##_##subtype##_##name), i);                \
	ktile_fpga_wrapper_##type##_##subtype##_##name.offset = p_ktile_##type##_##subtype##_##name->offset;                                    \
	OPENCL_ASSERT_OP(clSetKernelArg(kernel, arg_pos++, sizeof(KHitTile_fpga_wrapper), &ktile_fpga_wrapper_##type##_##subtype##_##name));    \
	OPENCL_ASSERT_OP(clSetKernelArg(kernel, arg_pos++, sizeof(cl_mem), (cl_mem *)(p_ktile_##type##_##subtype##_##name->data)));

#define CTRL_PARSE_ARGS_OUT(kernel, arg_idx, type, name) \
	CTRL_PARSE_ARGS_IN(kernel, arg_idx, type, name)

#define CTRL_PARSE_ARGS_IO(kernel, arg_idx, type, name) \
	CTRL_PARSE_ARGS_IN(kernel, arg_idx, type, name)

#define CTRL_PARSE_ARGS_INVAL(kernel, arg_idx, type, name)                                                             \
	OPENCL_ASSERT_OP(clSetKernelArg(kernel, arg_pos++,                                                                 \
									request.fpga.p_displacements[arg_idx + 1] - request.fpga.p_displacements[arg_idx], \
									((uint8_t *)args_list + request.fpga.p_displacements[arg_idx])));

/**
 * Defines stuff needed for a \e FPGALIB type kernel launch from the information passed by the user on the kernel definition.
 * @hideinitializer
 *
 * @param name Name of the kernel.
 * @param type Type of the kernel.
 * @param subtype Subtype of the kernel.
 * @param ... Parameters to the kernel and kernel body.
 *
 * @see Ctrl_ImplType, CTRL_KERNEL, CTRL_KERNEL_WRAP_FPGALIB
 * @todo FPGA lib kernels are not fully implemented
 */
#define CTRL_KERNEL_FPGALIB(name, type, subtype, ...)                                          \
	void Ctrl_Kernel_FPGA_##type##_##subtype##_##name(CTRL_KERNEL_EXTRACT_ARGS(__VA_ARGS__)) { \
		CTRL_KERNEL_EXTRACT_KERNEL_NO_STR(__VA_ARGS__)                                         \
	}

#define CTRL_KERNEL_OVERRIDE_THREADS_TASK    CTRL_THREAD_NULL
#define CTRL_KERNEL_OVERRIDE_THREADS_NDRANGE threads
#ifdef _CTRL_DEBUG_
#define CTRL_KERNEL_OVERRIDE_THREADS_INFO                                                                                       \
	if (threads.dims != 0 && new_threads.dims == 0) {                                                                           \
		fprintf(stderr,                                                                                                         \
				"[CTRL_KERNEL_WRAP_FPGA] Info: Launching task kernel with non-null thread space. Overriding to one thread.\n"); \
		fflush(stderr);                                                                                                         \
	}
#else // !_CTRL_DEBUG_
#define CTRL_KERNEL_OVERRIDE_THREADS_INFO
#endif // _CTRL_DEBUG_

#define CTRL_KERNEL_WRAP_FPGA(name, args_list, type, subtype, n_args, ...)                                                             \
	{                                                                                                                                  \
		int arg_pos = 0;                                                                                                               \
		OPENCL_ASSERT_OP(clSetKernelArg(ctrl_kernel_fpga_##type##_##subtype##_##name.p_kernel[request.fpga.type_id], arg_pos++,        \
										sizeof(Ctrl_Thread), &threads));                                                               \
		CTRL_PARSE_ARGS(ctrl_kernel_fpga_##type##_##subtype##_##name.p_kernel[request.fpga.type_id], n_args, __VA_ARGS__)              \
		Ctrl_Thread new_threads = CTRL_KERNEL_OVERRIDE_THREADS_##subtype;                                                              \
		CTRL_KERNEL_OVERRIDE_THREADS_INFO;                                                                                             \
		threads = new_threads;                                                                                                         \
		if (blocksize.dims != 0 && threads.dims != 0) {                                                                                \
			size_t global_size[3] = {1, 1, 1};                                                                                         \
			size_t local_size[3]  = {1, 1, 1};                                                                                         \
			switch (threads.dims) {                                                                                                    \
				case 3:                                                                                                                \
					global_size[0] = ((threads.k + blocksize.k - 1) / blocksize.k) * blocksize.k;                                      \
					global_size[1] = ((threads.j + blocksize.j - 1) / blocksize.j) * blocksize.j;                                      \
					global_size[2] = ((threads.i + blocksize.i - 1) / blocksize.i) * blocksize.i;                                      \
					local_size[0]  = blocksize.k;                                                                                      \
					local_size[1]  = blocksize.j;                                                                                      \
					local_size[2]  = blocksize.i;                                                                                      \
					break;                                                                                                             \
				case 2:                                                                                                                \
					global_size[0] = ((threads.j + blocksize.j - 1) / blocksize.j) * blocksize.j;                                      \
					global_size[1] = ((threads.i + blocksize.i - 1) / blocksize.i) * blocksize.i;                                      \
					local_size[0]  = blocksize.j;                                                                                      \
					local_size[1]  = blocksize.i;                                                                                      \
					break;                                                                                                             \
				case 1:                                                                                                                \
					global_size[0] = ((threads.i + blocksize.i - 1) / blocksize.i) * blocksize.i;                                      \
					local_size[0]  = blocksize.i;                                                                                      \
					break;                                                                                                             \
				default:                                                                                                               \
					fprintf(stderr, "[CTRL_KERNEL_WRAP_FPGA] ERROR: Invalid number of dimensions for thread space on kernel %s: %d\n", \
							#name, threads.dims);                                                                                      \
					exit(EXIT_FAILURE);                                                                                                \
			}                                                                                                                          \
			OPENCL_ASSERT_OP(clEnqueueNDRangeKernel(*(request.fpga.queue),                                                             \
													ctrl_kernel_fpga_##type##_##subtype##_##name.p_kernel[request.fpga.type_id],       \
													threads.dims, 0, global_size, local_size,                                          \
													0, NULL, request.fpga.p_last_kernel_event));                                       \
		} else {                                                                                                                       \
			OPENCL_ASSERT_OP(clEnqueueTask(*(request.fpga.queue),                                                                      \
										   ctrl_kernel_fpga_##type##_##subtype##_##name.p_kernel[request.fpga.type_id],                \
										   0, NULL, request.fpga.p_last_kernel_event));                                                \
		}                                                                                                                              \
		OPENCL_ASSERT_OP(clFlush(*(request.fpga.queue)));                                                                              \
	}

/**
 * Block of code that launches a \e FPGALIB kernel, this uses stuff defined on \e CTRL_KERNEL_FPGALIB.
 * @hideinitializer
 *
 * @param name Kernel name.
 * @param args_list List of arguments passed inside task when launching a kernel.
 * @param type Type of the kernel.
 * @param subtype Subtype of the kernel.
 * @param ... Parameters to the kernel and kernel body.
 *
 * @see CTRL_KERNEL_FPGALIB
 */
#define CTRL_KERNEL_WRAP_FPGALIB(name, args_list, type, subtype, ...)                                                         \
	{                                                                                                                         \
		Ctrl_Kernel_FPGA_##type##_##subtype##_##name(CTRL_KERNEL_ARG_LIST_ACCESS_KTILE(args_list, __VA_ARGS__));              \
		/* TODO @sergioalo: retain needed because rn this does not generate a new clevent, proper lib implementations will */ \
		clRetainEvent(*request.fpga.p_last_kernel_event);                                                                     \
	};

/**
 * Declares variables related to this kernel's info.
 * @hideinitializer
 *
 * @param name Name of the kernel.
 * @param type Type of this implementation.
 * @param subtype Subtype of this implementation.
 * @param ... Arguments recieved by the kernel (with roles).
 */
#define CTRL_KERNEL_DECLARATION_FPGA(name, type, subtype, ...)                                      \
	Ctrl_FPGA_KernelParams ctrl_kernel_fpga_##type##_##subtype##_##name = (Ctrl_FPGA_KernelParams){ \
		.p_kernel      = NULL,                                                                      \
		.p_kernel_name = CTRL_MACRO_STRINGIFY(ctrl_kernel_fpga_##type##_##subtype##_##name),        \
		.p_next        = NULL};                                                                            \
                                                                                                    \
	__attribute__((constructor)) static void Ctrl_FPGA_InitKernel_##type##_##subtype##_##name() {   \
		Ctrl_FPGA_KernelParams *curr_k_par = &FPGA_initial_kp;                                      \
		while (curr_k_par->p_next != NULL)                                                          \
			curr_k_par = curr_k_par->p_next;                                                        \
		curr_k_par->p_next = &ctrl_kernel_fpga_##type##_##subtype##_##name;                         \
	}

/**
 * Kernel function prototype for \e FPGALIB type kernels to allow moving kernel definitions to another file.
 * @hideinitializer
 *
 * @param name Name of the kernel.
 * @param type Type of this implementation.
 * @param subtype Subtype of this implementation.
 * @param n_params Number of arguments recieved by the kernel.
 * @param ... Arguments recieved by the kernel (with roles).
 */
#define CTRL_KERNEL_DECLARATION_FPGALIB(name, type, subtype, n_params...)

///@endcond
#endif //_CTRL_FPGA_KERNELPROTO_H_
