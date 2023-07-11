#ifndef _CTRL_PYNQ_KERNELPROTO_H_
#define _CTRL_PYNQ_KERNELPROTO_H_
///@cond INTERNAL
/**
 * @file Ctrl_PYNQ_KernelProto.h
 * @author Trasgo Group
 * @brief Macros to generate the code and manage PYNQ kernels.
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

#include <stdio.h>

#include "Kernel/Ctrl_KernelArgs.h"
#include "Kernel/Ctrl_Thread.h"

extern int last_swap_id;

#define PARSE_NO_FLOAT_ARG NO_FLOAT_TO_FLOAT(
#define NO_FLOAT_TO_FLOAT( arg ) struct context * context, int * reset, int * return_var, int host_id
#define PARSE_float LEAVE_FLOAT(
#define LEAVE_FLOAT( arg ) float arg, struct context * context, int * reset, int * return_var, int host_id

#define CTRL_PARSE_ARGS_NO_FLOAT(...) _CTRL_PARSE_ARGS_NO_FLOAT(COUNT(__VA_ARGS__), __VA_ARGS__)
#define _CTRL_PARSE_ARGS_NO_FLOAT(N, ...) __CTRL_PARSE_ARGS_NO_FLOAT(N, __VA_ARGS__)
#define __CTRL_PARSE_ARGS_NO_FLOAT(N, ...) CTRL_PARSE_ARGS_NO_FLOAT_##N(__VA_ARGS__)

#define CTRL_PARSE_ARGS_NO_FLOAT_21(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_20(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_20(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_19(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_19(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_18(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_18(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_17(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_17(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_16(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_16(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_15(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_15(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_14(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_14(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_13(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_13(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_12(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_12(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_11(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_11(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_10(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_10(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_9(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_9(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_8(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_8(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_7(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_7(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_6(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_6(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_5(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_5(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_4(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_4(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_3(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_3(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_2(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_2(arg, ...) arg, CTRL_PARSE_ARGS_NO_FLOAT_1(__VA_ARGS__)
#define CTRL_PARSE_ARGS_NO_FLOAT_1(arg) PARSE_##arg )


#ifndef _CTRL_PYNQ_KERNEL_FILE_
#include "pynq_api.h"
#endif // _CTRL_PYNQ_KERNEL_FILE_

#if defined(_CTRL_PYNQ_KERNEL_FILE_) && !defined(_CTRL_PYNQ_HOST_)
#define CTRL_KERNEL_FUNCTION_PYNQ( name, type, subtype, ... ) \
    void name(__VA_ARGS__)
#else
#define CTRL_KERNEL_FUNCTION_PYNQ( name, type, subtype, ... ) \
    void Ctrl_Host_Task_##name(CTRL_PARSE_ARGS_NO_FLOAT(__VA_ARGS__))
#endif

//Ctrl_Task Ctrl_KernelTaskCreate_Swap(int rr, char * bitstream);

/**
 * @author: Gabriel Rodriguez-Canal
 * @brief: Macros for the generation of the kernel signature and interface pragmas
 */
#define NO_POINTER_ARG volatile int * no_p_arg
#define NO_INT_ARG int no_int_arg
#if defined(_CTRL_PYNQ_KERNEL_FILE_) && !defined(_CTRL_PYNQ_HOST_)
#define NO_FLOAT_ARG float no_float_arg
#endif // _CTRL_PYNQ_KERNEL_FILE_

#define NO_POINTER_INTERFACE no_p_arg
#define NO_INT_INTERFACE no_int_arg
#define NO_FLOAT_INTERFACE no_float_arg

#if defined(_CTRL_PYNQ_KERNEL_FILE_) && !defined(_CTRL_PYNQ_HOST_)
#define CTRL_KERNEL_PYNQ_PARSE_KTILE( arg ) fpga_wrapper_##arg##_wrapper, data_##arg##_data
#define KTILE_ARGS(...) _KTILE_ARGS(__VA_ARGS__, KHitTile_int ktile_args_0, KHitTile_int ktile_args_1, KHitTile_int p_args_2)
#define _KTILE_ARGS(_1, _2, _3, ...)  \
    CTRL_KERNEL_PYNQ_PARSE_KTILE(_1), \
    CTRL_KERNEL_PYNQ_PARSE_KTILE(_2), \
    CTRL_KERNEL_PYNQ_PARSE_KTILE(_3)
#else
#define CTRL_KERNEL_PYNQ_PARSE_KTILE( arg ) to_HitTile_##arg

#define KTILE_ARGS3(arg, ...) CTRL_KERNEL_PYNQ_PARSE_KTILE(arg), KTILE_ARGS2(__VA_ARGS__)
#define KTILE_ARGS2(arg, ...) CTRL_KERNEL_PYNQ_PARSE_KTILE(arg), KTILE_ARGS1(__VA_ARGS__)
#define KTILE_ARGS1(arg) CTRL_KERNEL_PYNQ_PARSE_KTILE(arg)

#define KTILE_ARGS(...) _KTILE_ARGS(COUNT(__VA_ARGS__), __VA_ARGS__)
#define _KTILE_ARGS(N, ...) __KTILE_ARGS(N, __VA_ARGS__) 
#define __KTILE_ARGS(N, ...) KTILE_ARGS##N(__VA_ARGS__) 
#endif // _CTRL_PYNQ_KERNEL_FILE_

#if defined(_CTRL_PYNQ_KERNEL_FILE_) && !defined(_CTRL_PYNQ_HOST_)
#define INT_ARGS(...) _INT_ARGS(__VA_ARGS__, int i_args_0, int i_args_1, int i_args_2, int i_args_3, int i_args_4, int i_args_5, int i_args_6, int i_args_7)
#define _INT_ARGS(_1, _2, _3, _4, _5, _6, _7, _8, ...) _1, _2, _3, _4, _5, _6, _7, _8

#define FLOAT_ARGS(...) _FLOAT_ARGS(__VA_ARGS__, float f_args_0, float f_args_1, float f_args_2, float f_args_3, float f_args_4, float f_args_5, float f_args_6, float f_args_7)
#define _FLOAT_ARGS(_1, _2, _3, _4, _5, _6, _7, _8, ...) _1, _2, _3, _4, _5, _6, _7, _8, volatile struct context * context, int * return_var
#else
#define INT_ARGS(...) __VA_ARGS__
#define FLOAT_ARGS(...) __VA_ARGS__
#endif // _CTRL_PYNQ_KERNEL_FILE_

#ifndef _CTRL_PYNQ_HOST_
#define DO_PRAGMA(x) _Pragma(#x)
#else
#define DO_PRAGMA(x)
#endif // _CTRL_PYNQ_HOST_

#define DEF_KTILE_INTERFACES(...) _DEF_KTILE_INTERFACES(__VA_ARGS__, ktile_args_0, ktile_args_1, ktile_args_2)
#define _DEF_KTILE_INTERFACES(_1, _2, _3, ...) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_1##_data bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_2##_data bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_3##_data bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_1##_wrapper bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_2##_wrapper bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_3##_wrapper bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE m_axi port=_1##_data depth=1052676 max_widen_bitwidth=32) \
    DO_PRAGMA(HLS INTERFACE m_axi port=_2##_data depth=1052676 max_widen_bitwidth=32) \
    DO_PRAGMA(HLS INTERFACE m_axi port=_3##_data depth=1052676 max_widen_bitwidth=32)

#define DEF_INT_INTERFACES(...) _DEF_INT_INTERFACES(__VA_ARGS__, i_args_0, i_args_1, i_args_2, i_args_3, i_args_4, i_args_5, i_args_6, i_args_7)
#define _DEF_INT_INTERFACES(_1, _2, _3, _4, _5, _6, _7, _8, ...) \
            DO_PRAGMA(HLS INTERFACE s_axilite port=_1 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_2 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_3 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_4 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_5 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_6 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_7 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_8 bundle=CTRL_BUS) 

#define DEF_FLOAT_INTERFACES(...) _DEF_FLOAT_INTERFACES(__VA_ARGS__, f_args_0, f_args_1, f_args_2, f_args_3, f_args_4, f_args_5, f_args_6, f_args_7)
#define _DEF_FLOAT_INTERFACES(_1, _2, _3, _4, _5, _6, _7, _8, ...) \
            DO_PRAGMA(HLS INTERFACE s_axilite port=_1 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_2 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_3 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_4 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_5 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_6 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_7 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE s_axilite port=_8 bundle=CTRL_BUS) \
    DO_PRAGMA(HLS INTERFACE bram port=context storage_type=RAM_1P)

#ifndef _CTRL_PYNQ_HOST_
#define DEF_RETURN_INTERFACE(...) DO_PRAGMA(HLS INTERFACE s_axilite port=return bundle=CTRL_BUS) \
                                  DO_PRAGMA(HLS INTERFACE s_axilite port=return_var bundle=CTRL_BUS) \
                                  int incr_var[N_SAVED_VARS];
#else
#define DEF_RETURN_INTERFACE(...) DO_PRAGMA(HLS INTERFACE s_axilite port=return bundle=CTRL_BUS) \
                                  DO_PRAGMA(HLS INTERFACE s_axilite port=return_var bundle=CTRL_BUS) \
                                  int incr_var[N_SAVED_VARS]; \
                                  int read_host_task_ready; \
                                  printf("Before host_task_ready\n"); \
                                  do { \
                                      _Pragma("omp atomic read") \
                                      read_host_task_ready = host_task_ready; \
                                  } while(read_host_task_ready == 0); \
                                  printf("After host_task_ready. evicted_id: %d, host_id: %d\n", evicted_host_task_id, host_id); \
                                  if(evicted_host_task_id == host_id) \
                                    return;
#endif // _CTRL_PYNQ_HOST

/*
 * @author: Gabriel Rodriguez-Canal
 * @brief: Assembles the ktile in the body of the function to prevent pointer to pointer errors in Vitis HLS.
 * Inherited from the FPGA backend
 */
#if defined(_CTRL_PYNQ_KERNEL_FILE_) && !defined(_CTRL_PYNQ_HOST_)
#define KTILE( name, type ) \
    KHitTile_##type name; \
    name.data = name##_data; \
    *((fpga_wrapper_KHitTile_##type *)&name.origAcumCard) = name##_wrapper; \
    name.data += name.offset;
#else
#define KTILE( name, type )
#endif // _CTRL_PYNQ_KERNEL_FILE_

/*
 * @author: Gabriel Rodriguez-Canal
 * @brief: Macros for context switching
 */
#define N_SAVED_VARS 10
#define BLOCK_SIZE 5000

#define M(x, ...) x; M2(__VA_ARGS__)
#define M2(x) x;



#define _COUNT(v1,v2,v3,v4,v5,v6,v7,v8,N,...) N 
#define COUNT(...) _COUNT(__VA_ARGS__, 8,7,6,5,4,3,2,1)
#define EXTRACT(x) x


#define GENERATE_INDICES(...) _GENERATE_INDICES(EXTRACT(COUNT(__VA_ARGS__)), __VA_ARGS__)
#define _GENERATE_INDICES(N, ...) __GENERATE_INDICES(N, __VA_ARGS__)
#define __GENERATE_INDICES(N, ...) GENERATE_INDICES##N(__VA_ARGS__)

#define GENERATE_INDICES8(ind, ...) static int ind##_id = 7; \
                                    if(context->saved[ind##_id]) \
                                        ind = context->var[ind##_id]; \
                                    GENERATE_INDICES7(__VA_ARGS__)
#define GENERATE_INDICES7(ind, ...) static int ind##_id = 6; \
                                    if(context->saved[ind##_id]) \
                                        ind = context->var[ind##_id]; \
                                    GENERATE_INDICES6(__VA_ARGS__)
#define GENERATE_INDICES6(ind, ...) static int ind##_id = 5; \
                                    if(context->saved[ind##_id]) \
                                        ind = context->var[ind##_id]; \
                                    GENERATE_INDICES5(__VA_ARGS__)
#define GENERATE_INDICES5(ind, ...) static int ind##_id = 4; \
                                    if(context->saved[ind##_id]) \
                                        ind = context->var[ind##_id]; \
                                    GENERATE_INDICES4(__VA_ARGS__)
#define GENERATE_INDICES4(ind, ...) static int ind##_id = 3; \
                                    if(context->saved[ind##_id]) \
                                        ind = context->var[ind##_id]; \
                                    GENERATE_INDICES3(__VA_ARGS__)
#define GENERATE_INDICES3(ind, ...) static int ind##_id = 2; \
                                    if(context->saved[ind##_id]) \
                                        ind = context->var[ind##_id]; \
                                    GENERATE_INDICES2(__VA_ARGS__)
#define GENERATE_INDICES2(ind, ...) static int ind##_id = 1; \
                                    if(context->saved[ind##_id]) \
                                        ind = context->var[ind##_id]; \
                                    GENERATE_INDICES1(__VA_ARGS__)
#define GENERATE_INDICES1(ind) static int ind##_id = 0; \
                                    if(context->saved[ind##_id]) \
                                        ind = context->var[ind##_id]; 


struct context {
    //Pointers to the loop index variables and the integer variables. Used for the user to record
    //for which variables the context should be saved when they call the macro save_context().
    int var[N_SAVED_VARS];
    int init_var[N_SAVED_VARS]; // initial value of variable
    int incr_var[N_SAVED_VARS]; // increment of the loop variable
    int saved[N_SAVED_VARS];
    int valid; // set to 0 if the kernel was interrupted in the middle of a saving context operation. 1 otherwise
};


#define context_vars(...) \
    GENERATE_INDICES(__VA_ARGS__)


#define save_context(...) _save_context(EXTRACT(COUNT(__VA_ARGS__)), __VA_ARGS__)
#define _save_context(N, ...) __save_context(N, __VA_ARGS__)
#define __save_context(N, ...) save_context##N(__VA_ARGS__)

#if defined(_CTRL_PYNQ_KERNEL_FILE_) && !defined(_CTRL_PYNQ_HOST_)
#define save_context8(v, ...) context->var[v##_id] = v; \
                                ap_wait(); \
                                context->saved[v##_id] = 1; \
                                ap_wait(); \
                                save_context7(__VA_ARGS__)
#define save_context7(v, ...) context->var[v##_id] = v; \
                                ap_wait(); \
                                context->saved[v##_id] = 1; \
                                ap_wait(); \
                                save_context6(__VA_ARGS__)
#define save_context6(v, ...) context->var[v##_id] = v; \
                                ap_wait(); \
                                context->saved[v##_id] = 1; \
                                ap_wait(); \
                                save_context5(__VA_ARGS__)
#define save_context5(v, ...) context->var[v##_id] = v; \
                                ap_wait(); \
                                context->saved[v##_id] = 1; \
                                ap_wait(); \
                                save_context4(__VA_ARGS__)
#define save_context4(v, ...) context->var[v##_id] = v; \
                                ap_wait(); \
                                context->saved[v##_id] = 1; \
                                ap_wait(); \
                                save_context3(__VA_ARGS__)
#define save_context3(v, ...) context->var[v##_id] = v; \
                                ap_wait(); \
                                context->saved[v##_id] = 1; \
                                ap_wait(); \
                                save_context2(__VA_ARGS__)
#define save_context2(v, ...) context->var[v##_id] = v; \
                                ap_wait(); \
                                context->saved[v##_id] = 1; \
                                ap_wait(); \
                                save_context1(__VA_ARGS__)
#define save_context1(v, ...) context->var[v##_id] = v; \
                                ap_wait(); \
                                context->saved[v##_id] = 1; \
                                ap_wait();
#else
#define save_context8(v, ...) context->var[v##_id] = v; \
                                context->saved[v##_id] = 1; \
                                save_context7(__VA_ARGS__)
#define save_context7(v, ...) context->var[v##_id] = v; \
                                context->saved[v##_id] = 1; \
                                save_context6(__VA_ARGS__)
#define save_context6(v, ...) context->var[v##_id] = v; \
                                context->saved[v##_id] = 1; \
                                save_context5(__VA_ARGS__)
#define save_context5(v, ...) context->var[v##_id] = v; \
                                context->saved[v##_id] = 1; \
                                save_context4(__VA_ARGS__)
#define save_context4(v, ...) context->var[v##_id] = v; \
                                context->saved[v##_id] = 1; \
                                save_context3(__VA_ARGS__)
#define save_context3(v, ...) context->var[v##_id] = v; \
                                context->saved[v##_id] = 1; \
                                save_context2(__VA_ARGS__)
#define save_context2(v, ...) context->var[v##_id] = v; \
                                context->saved[v##_id] = 1; \
                                save_context1(__VA_ARGS__)
#define save_context1(v, ...) context->var[v##_id] = v; \
                                context->saved[v##_id] = 1;
#endif // _CTRL_PYNQ_KERNEL_FILE_

#define retrieve_context(...) context->valid = 0; \
                              _retrieve_context(EXTRACT(COUNT(__VA_ARGS__)), __VA_ARGS__)
#define _retrieve_context(N, ...) __retrieve_context(N, __VA_ARGS__)
#define __retrieve_context(N, ...) retrieve_context##N(__VA_ARGS__)

#define retrieve_context10( v, ... ) if(context->saved[v##_id]) \
                                        v = context->var[v##_id]; \
                                     else \
                                        v = context->init_var[v##_id]; \
                                     retrieve_context9( __VA_ARGS__ )
#define retrieve_context9( v, ... ) if(context->saved[v##_id]) \
                                        v = context->var[v##_id]; \
                                     else \
                                        v = context->init_var[v##_id]; \
                                     retrieve_context8( __VA_ARGS__ )
#define retrieve_context8( v, ... ) if(context->saved[v##_id]) \
                                        v = context->var[v##_id]; \
                                     else \
                                        v = context->init_var[v##_id]; \
                                     retrieve_context7( __VA_ARGS__ )
#define retrieve_context7( v, ... ) if(context->saved[v##_id]) \
                                        v = context->var[v##_id]; \
                                     else \
                                        v = context->init_var[v##_id]; \
                                     retrieve_context6( __VA_ARGS__ )
#define retrieve_context6( v, ... ) if(context->saved[v##_id]) \
                                        v = context->var[v##_id]; \
                                     else \
                                        v = context->init_var[v##_id]; \
                                     retrieve_context5( __VA_ARGS__ )
#define retrieve_context5( v, ... ) if(context->saved[v##_id]) \
                                        v = context->var[v##_id]; \
                                     else \
                                        v = context->init_var[v##_id]; \
                                     retrieve_context4( __VA_ARGS__ )
#define retrieve_context4( v, ... ) if(context->saved[v##_id]) \
                                        v = context->var[v##_id]; \
                                     else \
                                        v = context->init_var[v##_id]; \
                                     retrieve_context3( __VA_ARGS__ )
#define retrieve_context3( v, ... ) if(context->saved[v##_id]) \
                                        v = context->var[v##_id]; \
                                     else \
                                        v = context->init_var[v##_id]; \
                                     retrieve_context2( __VA_ARGS__ )
#define retrieve_context2( v, ... ) if(context->saved[v##_id]) \
                                        v = context->var[v##_id]; \
                                     else \
                                        v = context->init_var[v##_id]; \
                                     retrieve_context1( __VA_ARGS__ )
#define retrieve_context1( v, ... ) if(context->saved[v##_id]) \
                                        v = context->var[v##_id]; \
                                     else \
                                        v = context->init_var[v##_id]; \
                                     context->valid = 1;

// Used to increase the value of a saved variable. For example, when all the inner iterations have been completed but the outer trip index was saved before
#define incr_context(v) context->var[v##_id]++; 


#define for_save( loop_var, init_val, stop_val, incr ) \
    context->valid = 0; \
    context->init_var[loop_var##_id] = init_val; \
    retrieve_context( loop_var ); \
    context->incr_var[loop_var##_id] = incr; \
    incr_var[loop_var##_id] = incr; \
    context->valid = 1; \
    DO_PRAGMA(HLS pipeline off) \
    for(; loop_var < stop_val; loop_var += incr)



#define save_context_if_vars(...) _save_context_if_vars(COUNT(__VA_ARGS__), __VA_ARGS__)
#define _save_context_if_vars(N, ...) __save_context_if_vars(N, __VA_ARGS__)
#define __save_context_if_vars(N, ...) save_context_if_vars##N(__VA_ARGS__)

#define save_context_if_vars1(loop_var)
#define save_context_if_vars2(loop_var, ...) save_context(__VA_ARGS__)
#define save_context_if_vars3(loop_var, ...) save_context(__VA_ARGS__)
#define save_context_if_vars4(loop_var, ...) save_context(__VA_ARGS__)
#define save_context_if_vars5(loop_var, ...) save_context(__VA_ARGS__)
#define save_context_if_vars6(loop_var, ...) save_context(__VA_ARGS__)
#define save_context_if_vars7(loop_var, ...) save_context(__VA_ARGS__)
#define save_context_if_vars8(loop_var, ...) save_context(__VA_ARGS__)
#define save_context_if_vars9(loop_var, ...) save_context(__VA_ARGS__)


#if defined(_CTRL_PYNQ_KERNEL_FILE_) && !defined(_CTRL_PYNQ_HOST_)
#define for_checkpoint( ... ) \
    { \
    DO_PRAGMA(HLS protocol fixed) \
    context->valid = 0; \
    ap_wait(); \
    ap_wait(); \
    save_context_if_vars(__VA_ARGS__); \
    _for_checkpoint(__VA_ARGS__)


#define _for_checkpoint( loop_var, ... ) \
    context->var[loop_var##_id] = loop_var + incr_var[loop_var##_id]; \
    ap_wait(); \
    ap_wait(); \
    context->saved[loop_var##_id] = 1; \
    ap_wait(); \
    context->valid = 1; \
    } \
    } \
    { \
    DO_PRAGMA(HLS protocol fixed) \
    loop_var = context->init_var[loop_var##_id]; \
    ap_wait(); \
    ap_wait(); \
    context->valid = 0; \
    ap_wait(); \
    save_context(loop_var); \
    context->valid = 1; \
    } \
    {
#else
#define for_checkpoint( ... ) \
    { \
    context->valid = 0; \
    save_context_if_vars(__VA_ARGS__); \
    _for_checkpoint(__VA_ARGS__)

#define _for_checkpoint( loop_var, ... ) \
    context->var[loop_var##_id] = loop_var + context->incr_var[loop_var##_id]; \
    context->saved[loop_var##_id] = 1; \
    context->valid = 1; \
    } \
    if(*reset) { *reset = -1; printf("RESET CPU!!!\n"); return; } \
    } \
    { \
    DO_PRAGMA(HLS protocol fixed) \
    loop_var = context->init_var[loop_var##_id]; \
    context->valid = 0; \
    save_context(loop_var); \
    context->valid = 1; \
    } \
    {
#endif // _CTRL_PYNQ_KERNEL_FILE_ 

#define NO_RETURN 0

#if defined(_CTRL_PYNQ_KERNEL_FILE_) && !defined(_CTRL_PYNQ_HOST_)
#define ctrl_return( var ) *return_var = var;
#else
#define ctrl_return( var ) *return_var = var; \
        PYNQ_writeGPIO(&host_interrupt, &enable); 
#endif // _CTRL_PYNQ_KERNEL_FILE_



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SCHEDULER COMPONENTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TODO: This must be a global variable shared with Ctrl_Pynq.c
#define N_RR 2

#define MAX_N_TASKS 100

#define ANY_RR -1

#define LAUNCH_TYPE 0
#define RECONFIG_TYPE 1

int generated_interrupts[N_RR+1];

typedef struct Task {
    int id; 
    int type;
    void * ext_task;
} t_Task;

typedef struct ReconfigTask {
    int partial;
    char * bitstream_path;
} t_ReconfigTask;

typedef struct LaunchTask {
    int iters;
} t_LaunchTask;

t_Task TaskQueue[N_RR][MAX_N_TASKS];
int queue_insert_point[N_RR];
int queue_head[N_RR];


// ADDRESSES
#define INTERRUPT_CTRL_ADDRESS 0x41200000
#define DMA_CTRL_ADDRESS 0x41E00000

#define N_P_ARGS 3
#define N_INT_ARGS 8
#define N_FLOAT_ARGS 8

// The index refers to the RR
int bram_ctrl_address[N_RR];
int rr_address[N_RR];

// Arguments offset in CTRL BUS
int ktile_wrapper_args_offset[N_P_ARGS];
int ktile_data_args_offset[N_P_ARGS];
int int_args_offset[N_INT_ARGS];
int float_args_offset[N_FLOAT_ARGS];
int return_offset;

typedef volatile int * data_KHitTile_int;

#define MM_AXI_SIZE 65536
#define BRAM_SIZE 8192

// IP's addresses
#define IP_CTRL                 0x00
#define AP_START                0x1
#define GIER                    0x04
#define IP_IER                  0x08
#define IP_ISR                  0x0C
#define INTRPT_AP_DONE          0x1
#define INTRPT_AP_READY         0x2
#define OUT_REG                 0x10
#define INP_REG                 0x20
#define _INTRPT                 INTRPT_AP_DONE
#define IN_ARRAY                0x18
#define OUT_ARRAY               0x24
#define ITERS                   0x30


typedef struct InterruptManager {
    PYNQ_AXI_INTERRUPT_CONTROLLER interrupt_controller;
    PYNQ_UIO uio_state;
} t_InterruptManager;

typedef struct DMAController {
    PYNQ_AXI_DMA state;
} t_DMAController;

t_InterruptManager InterruptManager;

t_DMAController DMAController;

#define RR_SWAPPING
#define RR_AVAILABLE
#define RR_RUNNING

typedef struct ReconfigurableRegion {
    PYNQ_HLS state;

    PYNQ_SHARED_MEMORY ktile_data[N_P_ARGS];

    //int * d[N_P_ARGS];

    PYNQ_GPIO decouple;
    PYNQ_GPIO decouple_status;
    PYNQ_GPIO reset;
    PYNQ_GPIO decouple_rst_clk_gpio;

    struct context context;
    int return_var;
    int running_task_id;

    int busy;
} t_ReconfigurableRegion;

// GPIO line that toggles an interrupt in the CPU. Used to detect the end of a host-task
PYNQ_GPIO host_interrupt;

typedef struct BRAMController {
    PYNQ_MMIO_WINDOW bram;
} t_BRAMController;

t_ReconfigurableRegion RR[N_RR+1];

t_BRAMController BRAMController[N_RR];

void setup_scheduler(int restart_tasks);
void setup_reconfigurable_region(int rr);
void reset_interrupt_controller();
#include <sys/time.h>
int wait_finish(struct timeval * timeout);
void clean();
void setup_dma();
int get_return(int rr);
void clean_bram(int rr);
void reset(int rr);
void setup_hls_interrupts(int rr);
void _print_output(int rr);


/* @author: Gabriel Rodriguez-Canal \
   @brief: CTRL_PYNQ_PARSE_ARGS generates the arguments passed to the kernel. Recall they must be passed by value in the case of INVAL while \
   IN and OUT must be split in a wrapper and a pointer to data. The wrapper will be of the form fpga_wrapper_KHitTile_<type> <name>_wrapper. \
   This macro is leveraged to have both the type of the wrapper and the index of the displacement at the moment of the creation of the arguments. \
   Take into account the macro arguments type and name are referring to the type and the name of the kernel argument and not those of the kernel \
   as in the case of CTRL_KERNEL_WRAP_FPGA. For that reason, the kernel name must also be passed. \
*/ \
#define CTRL_PYNQ_PARSE_ARGS( n_args, rr, ... ) \
    _CTRL_PYNQ_PARSE_ARGS( n_args, rr, __VA_ARGS__ )

#define _CTRL_PYNQ_PARSE_ARGS( n_args, rr, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##n_args( rr, 0, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_1( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \

#define CTRL_PYNQ_PARSE_ARGS_2( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_1( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_3( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_2( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_4( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_3( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_5( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_4( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_6( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_5( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_7( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_6( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_8( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_7( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_9( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_8( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_10( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_9( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_11( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_10( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_12( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_11( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_13( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_12( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_14( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_13( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_15( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_14( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_16( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_15( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_17( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_16( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_18( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_17( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_19( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_18( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_20( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_19( rr, arg_idx+1, __VA_ARGS__ )

#define CTRL_PYNQ_PARSE_ARGS_21( rr, arg_idx, role, type, name, ... ) \
	CTRL_PYNQ_PARSE_ARGS_##role( rr, arg_idx, type, name ) \
	CTRL_PYNQ_PARSE_ARGS_20( rr, arg_idx+1, __VA_ARGS__ )


#define CTRL_PYNQ_PARSE_ARGS_IN( rr, arg_idx, type, name ) \
    KHitTile *p_ktile_##type##_##subtype##_##name = (KHitTile *)((uint8_t *)args_list + request.pynq.p_displacements[arg_idx]); \
    fpga_wrapper_K##type ktile_fpga_wrapper_##type##_##subtype##_##name = { .origAcumCard = { p_ktile_##type##_##subtype##_##name->origAcumCard[0], \
            p_ktile_##type##_##subtype##_##name->origAcumCard[1], p_ktile_##type##_##subtype##_##name->origAcumCard[2], p_ktile_##type##_##subtype##_##name->origAcumCard[3] }, \
            .card = { p_ktile_##type##_##subtype##_##name->card[0], p_ktile_##type##_##subtype##_##name->card[1], p_ktile_##type##_##subtype##_##name->card[2] }, \
            .offset = p_ktile_##type##_##subtype##_##name->offset }; \
    PYNQ_writeToHLS(&RR[rr].state, &ktile_fpga_wrapper_##type##_##subtype##_##name, ktile_wrapper_args_offset[ktile_arg_pos], sizeof(fpga_wrapper_KHitTile_int)); \
    PYNQ_writeToHLS(&RR[rr].state, &p_ktile_##type##_##subtype##_##name->pynq_shared_memory.physical_address, ktile_data_args_offset[ktile_arg_pos], sizeof(volatile int *)); \
    ktile_arg_pos++;

#define CTRL_PYNQ_PARSE_ARGS_OUT( rr, arg_idx, type, name ) \
    CTRL_PYNQ_PARSE_ARGS_IN( rr, arg_idx, type, name )

#define CTRL_PYNQ_PARSE_ARGS_IO( rr, arg_idx, type, name ) \
    CTRL_PYNQ_PARSE_ARGS_IN( rr, arg_idx, type, name )


#define CTRL_PYNQ_PARSE_ARGS_INVAL( rr, arg_idx, type, name ) \
    PYNQ_writeToHLS(&RR[rr].state, ((uint8_t *)args_list + request.pynq.p_displacements[arg_idx]), type##_args_offset[type##_arg_pos], request.pynq.p_displacements[arg_idx+1] - request.pynq.p_displacements[arg_idx]); \
    type##_arg_pos++; 

/**
 * Defines the function containing the user provided code for a \e GENERIC type kernel
 * @hideinitializer
 *
 * @param name Name of the kernel.
 * @param type Type of the kernel.
 * @param subtype Subtype of the kernel.
 * @param ... Parameters to the kernel.
 *
 * @see Ctrl_ImplType, CTRL_KERNEL, CTRL_KERNEL_WRAP_PYNQ_GENERIC
 */
#define CTRL_KERNEL_PYNQ_GENERIC(name, type, subtype, ...) \
	CTRL_KERNEL_PYNQ(name, type, subtype, __VA_ARGS__)

/**
 * Defines the function containing the user provided code for a \e PYNQLIB type kernel
 * @hideinitializer
 *
 * @param name Name of the kernel.
 * @param type Type of the kernel.
 * @param subtype Subtype of the kernel.
 * @param ... Parameters to the kernel.
 *
 * @see Ctrl_ImplType, CTRL_KERNEL, CTRL_KERNEL_WRAP_PYNQLIB
 */
#define CTRL_KERNEL_PYNQLIB(name, type, subtype, ...) CTRL_KERNEL_PYNQLIB_##subtype(name, type, subtype, __VA_ARGS__)

/**
 * Defines the function containing the user provided code for a \e PYNQLIB type kernel
 * @hideinitializer
 *
 * @param name Name of the kernel.
 * @param type Type of the kernel.
 * @param subtype Subtype of the kernel.
 * @param ... Parameters to the kernel.
 *
 * @see Ctrl_ImplType, CTRL_KERNEL, CTRL_KERNEL_WRAP_PYNQLIB
 */
#define CTRL_KERNEL_FUNCTION_PYNQLIB(name, type, subtype, ...) CTRL_KERNEL_FUNCTION_PYNQLIB_##subtype(name, type, subtype, __VA_ARGS__)

#ifdef _CTRL_MKL_
/**
 * Defines the function containing the user provided code for a \e PYNQLIB_MKL type kernel
 * @hideinitializer
 *
 * @param name Name of the kernel.
 * @param type Type of the kernel.
 * @param subtype Subtype of the kernel.
 * @param ... Parameters to the kernel.
 *
 * @see Ctrl_ImplType, CTRL_KERNEL, CTRL_KERNEL_WRAP_PYNQLIB
 */
#define CTRL_KERNEL_PYNQLIB_MKL(name, type, subtype, ...)                                      \
	C_GUARD                                                                                   \
	void Ctrl_Kernel_PYNQ_##type##_##subtype##_##name(CTRL_KERNEL_EXTRACT_ARGS(__VA_ARGS__)) { \
		CTRL_KERNEL_EXTRACT_KERNEL_NO_STR(__VA_ARGS__)                                        \
	}
/**
 * Defines the function containing the user provided code for a \e PYNQLIB_MKL type kernel
 * @hideinitializer
 *
 * @param name Name of the kernel.
 * @param type Type of the kernel.
 * @param subtype Subtype of the kernel.
 * @param ... Parameters to the kernel.
 *
 * @see Ctrl_ImplType, CTRL_KERNEL_FUNCTION, CTRL_KERNEL_WRAP_PYNQLIB
 */
#define CTRL_KERNEL_FUNCTION_PYNQLIB_MKL(name, type, subtype, ...) \
	C_GUARD                                                       \
	void Ctrl_Kernel_PYNQ_##type##_##subtype##_##name(__VA_ARGS__)
#else // _CTRL_MKL_
#define CTRL_KERNEL_PYNQLIB_MKL(...)
#define CTRL_KERNEL_FUNCTION_PYNQLIB_MKL(...)
#endif // _CTRL_MKL_

/**
 * Block of code that launches a \e PYNQ kernel, this calculates thread ids and calls to the function defined in either
 * \e CTRL_KERNEL_PYNQ.
 * @hideinitializer
 *
 * @param name kernel name.
 * @param argsList list of arguments passed inside task when launching a kernel.
 * @param type Type of the kernel.
 * @param subtype Subtype of the kernel.
 * @param ... Parameters to the kernel.
 *
 * @pre A kernel of type \p type and subtype \p subtype must have been defined via \e CTRL_KERNEL.
 * @see CTRL_KERNEL_PYNQ
 */
#define CTRL_FILTER_AS_VA_ARGS( ... ) __VA_ARGS__
#define CTRL_N_ARGS_PLUS_1( n_args, ... ) CTRL_##n_args##_ARGS_PLUS_1( __VA_ARGS__ )
#define CTRL_1_ARGS_PLUS_1( ... ) 2, __VA_ARGS__
#define CTRL_2_ARGS_PLUS_1( ... ) 3, __VA_ARGS__
#define CTRL_3_ARGS_PLUS_1( ... ) 4, __VA_ARGS__
#define CTRL_4_ARGS_PLUS_1( ... ) 5, __VA_ARGS__
#define CTRL_5_ARGS_PLUS_1( ... ) 6, __VA_ARGS__
#define CTRL_6_ARGS_PLUS_1( ... ) 7, __VA_ARGS__
#define CTRL_7_ARGS_PLUS_1( ... ) 8, __VA_ARGS__
#define CTRL_8_ARGS_PLUS_1( ... ) 9, __VA_ARGS__
#define CTRL_9_ARGS_PLUS_1( ... ) 10, __VA_ARGS__
#define CTRL_10_ARGS_PLUS_1( ... ) 11, __VA_ARGS__
#define CTRL_11_ARGS_PLUS_1( ... ) 12, __VA_ARGS__
#define CTRL_12_ARGS_PLUS_1( ... ) 13, __VA_ARGS__
#define CTRL_13_ARGS_PLUS_1( ... ) 14, __VA_ARGS__
#define CTRL_14_ARGS_PLUS_1( ... ) 15, __VA_ARGS__
#define CTRL_15_ARGS_PLUS_1( ... ) 16, __VA_ARGS__
#define CTRL_16_ARGS_PLUS_1( ... ) 17, __VA_ARGS__
#define CTRL_17_ARGS_PLUS_1( ... ) 18, __VA_ARGS__
#define CTRL_18_ARGS_PLUS_1( ... ) 19, __VA_ARGS__
#define CTRL_19_ARGS_PLUS_1( ... ) 20, __VA_ARGS__
#define CTRL_20_ARGS_PLUS_1( ... ) 21, __VA_ARGS__
#define CTRL_21_ARGS_PLUS_1( ... ) 22, __VA_ARGS__
#define CTRL_22_ARGS_PLUS_1( ... ) 23, __VA_ARGS__
#define CTRL_23_ARGS_PLUS_1( ... ) 24, __VA_ARGS__
#define CTRL_24_ARGS_PLUS_1( ... ) 25, __VA_ARGS__
#define CTRL_25_ARGS_PLUS_1( ... ) 26, __VA_ARGS__

#define CTRL_KERNEL_WRAP_PYNQ( name, args_list, type, subtype, n_args, ... ) \
{ \
    int ktile_arg_pos = 0; \
    int int_arg_pos = 0; \
    int float_arg_pos = 0; \
    CTRL_PYNQ_PARSE_ARGS( n_args, request.pynq.rr, __VA_ARGS__) \
	RR[request.pynq.rr].running_task_id = request.pynq.launch_id; \
    fprintf(exp_file, "{ \"op\":\"launch_task\", \"task_name\":\"%s\", \"task_id\":%d, \"rr\": %d, \"start_time\":%lf }\n", #name, request.pynq.launch_id, request.pynq.rr, omp_get_wtime() - init_time, request.pynq.launch_id); \
        /* printf("=== LAUNCH KERNEL RR %d\n", request.pynq.rr); */ \
    PYNQ_writeToHLS(&RR[request.pynq.rr].state, &enable, 0, sizeof(int)); \
}

/**
 * Block of code that launches a \e GENERIC kernel on \e PYNQ architecture, this calculates thread ids and calls the
 * function defined in \e CTRL_KERNEL_PYNQ_GENERIC.
 * @hideinitializer
 *
 * @param name Kernel name.
 * @param argslist List of arguments passed inside task when launching a kernel.
 * @param type Type of the kernel.
 * @param subtype Subtype of the kernel.
 * @param ... Parameters to the kernel.
 *
 * @pre A kernel of type \p type and subtype \p subtype must have been defined via \e CTRL_KERNEL.
 * @see CTRL_KERNEL_PYNQ_GENERIC
 */
#define CTRL_KERNEL_WRAP_PYNQ_GENERIC(name, argslist, type, subtype, ...) \
	CTRL_KERNEL_WRAP_PYNQ(name, argslist, type, subtype, __VA_ARGS__)

/**
 * Block of code that launches a \e PYNQLIB kernel, this calls the function defined in \e CTRL_KERNEL_PYNQLIB.
 * @hideinitializer
 *
 * @param name Kernel name.
 * @param argslist List of arguments passed inside task when launching a kernel.
 * @param type Type of the kernel.
 * @param subtype Subtype of the kernel.
 * @param ... Parameters to the kernel.
 *
 * @pre A kernel of type \p type and subtype \p subtype must have been defined via \e CTRL_KERNEL.
 * @see CTRL_KERNEL_PYNQLIB
 */
#define CTRL_KERNEL_WRAP_PYNQLIB(name, argslist, type, subtype, ...) CTRL_KERNEL_WRAP_PYNQLIB_##subtype(name, argslist, type, subtype, __VA_ARGS__)

#ifdef _CTRL_MKL_
/**
 * Block of code that launches a \e PYNQLIB_MKL kernel, this calls the function defined in \e CTRL_KERNEL_PYNQLIB.
 * @hideinitializer
 *
 * @param name Kernel name.
 * @param argslist List of arguments passed inside task when launching a kernel.
 * @param type Type of the kernel.
 * @param subtype Subtype of the kernel.
 * @param ... Parameters to the kernel.
 *
 * @pre A kernel of type \p type and subtype \p subtype must have been defined via \e CTRL_KERNEL.
 * @see CTRL_KERNEL_PYNQLIB
 */
#define CTRL_KERNEL_WRAP_PYNQLIB_MKL(name, argslist, type, subtype, ...)                                        \
	{                                                                                                          \
		mkl_set_num_threads_local(request.cpu.n_cores);                                                        \
		Ctrl_Kernel_PYNQ_##type##_##subtype##_##name(CTRL_KERNEL_ARG_LIST_ACCESS_KTILE(argslist, __VA_ARGS__)); \
	};
#else // _CTRL_MKL_
#define CTRL_KERNEL_WRAP_PYNQLIB_MKL(...)
#endif // _CTRL_MKL_

/**
 * Kernel function prototype for \e PYNQ type kernels to allow moving kernel definitions to another file.
 *
 * @note Moving the kernel definition to a separate file is discouraged on \e PYNQ type kernels because of the inability of the
 * compiler to perform inlining on it, probably leading to poor performance.
 * @hideinitializer
 *
 * @param name Name of the kernel.
 * @param type Type of this implementation.
 * @param subtype Subtype of this implementation.
 * @param n_params Number of arguments recieved by the kernel.
 * @param ... Arguments recieved by the kernel (with roles).
 */
#define CTRL_KERNEL_DECLARATION_PYNQ(name, type, subtype, n_params, ...) \
	void Ctrl_Kernel_PYNQ_##type##_##subtype##_##name(Ctrl_Thread threads, Ctrl_Thread blocksize, int n_cores, CTRL_KERNEL_EXTRACT_DECLARATION_ARGS_##n_params(__VA_ARGS__), int dummy_arg);

/**
 * Kernel function prototype for \e PYNQLIB type kernels to allow moving kernel definitions to another file.
 * @hideinitializer
 *
 * @param name Name of the kernel.
 * @param type Type of this implementation.
 * @param subtype Subtype of this implementation.
 * @param n_params Number of arguments recieved by the kernel.
 * @param ... Arguments recieved by the kernel (with roles).
 */
#define CTRL_KERNEL_DECLARATION_PYNQLIB(name, type, subtype, n_params, ...) \
	CTRL_KERNEL_DECLARATION_PYNQLIB_##subtype(name, type, subtype, n_params, __VA_ARGS__)

#ifdef _CTRL_MKL_
/**
 * Kernel function prototype for \e PYNQLIB_MKL type kernels to allow moving kernel definitions to another file.
 * @hideinitializer
 *
 * @param name Name of the kernel.
 * @param type Type of this implementation.
 * @param subtype Subtype of this implementation.
 * @param n_params Number of arguments recieved by the kernel.
 * @param ... Arguments recieved by the kernel (with roles).
 */
#define CTRL_KERNEL_DECLARATION_PYNQLIB_MKL(name, type, subtype, n_params, ...) \
	void Ctrl_Kernel_PYNQ_##type##_##subtype##_##name(CTRL_KERNEL_EXTRACT_DECLARATION_ARGS_##n_params(__VA_ARGS__));
#else // _CTRL_MKL_
#define CTRL_KERNEL_DECLARATION_PYNQLIB_MKL(...)
#endif // _CTRL_MKL_

///@endcond
#endif //_CTRL_PYNQ_KERNELPROTO_H_
