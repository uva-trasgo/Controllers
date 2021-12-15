/*
 * CAL_KProto: Macros to generate the code, from the prototype declaration of a kernel,
 * 			of the enqueue/dequeue(launching) functions. The launching function is a
 * 			wrapper to invocate the actual kernel implementations
 *
 * @version 1.2
 * @author Ana Moreton-Fernandez
 * @date OCt 2017
 *
 * NOTE: Macros to support up to 7 different implementations, and up to 9 different parameters
 *		in a kernel call. Add more macros for higher number of implementations and/or parameters.
 */

#ifndef _CAL_KProto_
#define _CAL_KProto_

#include "CAL_KImpl.h"
#include "CAL_KXPhi.h"
#include "CAL_KArgs.h"
#include "CAL_TaskQueue.h"
#include "CAL_KernelGPUChar.h"
#include <cuda.h>
#include <assert.h>
#include <cuda_runtime.h>

/*
 *******************************************************************************************
 *******************************************************************************************
 *************************************  Common Macros  *************************************
 *******************************************************************************************
 *******************************************************************************************
 */


/* Skip the implementation names, and apply another macro to the parameter list */
#define CAL_PROTO_SKIP_IMPL( action, list, numImpl, ... )	CAL_PROTO_SKIP_IMPL_##numImpl( action, list, __VA_ARGS__ )
#define CAL_PROTO_SKIP_IMPL_1( action, list, impl, ... )	CAL_##action( list, __VA_ARGS__ )
#define CAL_PROTO_SKIP_IMPL_2( action, list, impl, ... )	CAL_PROTO_SKIP_IMPL_1( action, list, __VA_ARGS__ )
#define CAL_PROTO_SKIP_IMPL_3( action, list, impl, ... )	CAL_PROTO_SKIP_IMPL_2( action, list, __VA_ARGS__ )
#define CAL_PROTO_SKIP_IMPL_4( action, list, impl, ... )	CAL_PROTO_SKIP_IMPL_3( action, list, __VA_ARGS__ )
#define CAL_PROTO_SKIP_IMPL_5( action, list, impl, ... )	CAL_PROTO_SKIP_IMPL_4( action, list, __VA_ARGS__ )
#define CAL_PROTO_SKIP_IMPL_6( action, list, impl, ... )	CAL_PROTO_SKIP_IMPL_5( action, list, __VA_ARGS__ )
#define CAL_PROTO_SKIP_IMPL_7( action, list, impl, ... )	CAL_PROTO_SKIP_IMPL_6( action, list, __VA_ARGS__ )

/* Copy parameter roles, types and names */
#define CAL_PROTO_ROLED( list, numArgs, ... )	numArgs, CAL_PROTO_ROLED_##numArgs( __VA_ARGS__ )
#define CAL_PROTO_ROLED_1( role, type, name )		role, type, name
#define CAL_PROTO_ROLED_2( role, type, name, ... )	role, type, name, CAL_PROTO_ROLED_1( __VA_ARGS__ )
#define CAL_PROTO_ROLED_3( role, type, name, ... )	role, type, name, CAL_PROTO_ROLED_2( __VA_ARGS__ )
#define CAL_PROTO_ROLED_4( role, type, name, ... )	role, type, name, CAL_PROTO_ROLED_3( __VA_ARGS__ )
#define CAL_PROTO_ROLED_5( role, type, name, ... )	role, type, name, CAL_PROTO_ROLED_4( __VA_ARGS__ )
#define CAL_PROTO_ROLED_6( role, type, name, ... )	role, type, name, CAL_PROTO_ROLED_5( __VA_ARGS__ )
#define CAL_PROTO_ROLED_7( role, type, name, ... )	role, type, name, CAL_PROTO_ROLED_6( __VA_ARGS__ )
#define CAL_PROTO_ROLED_8( role, type, name, ... )	role, type, name, CAL_PROTO_ROLED_7( __VA_ARGS__ )
#define CAL_PROTO_ROLED_9( role, type, name, ... )	role, type, name, CAL_PROTO_ROLED_8( __VA_ARGS__ )

/* Copy parameter names */
#define CAL_PROTO_VALUE( list, numArgs2, ... )	CAL_PROTO_VALUE_##numArgs2( __VA_ARGS__ )
#define CAL_PROTO_VALUE_1( role, type, name )		 name
#define CAL_PROTO_VALUE_2( role, type, name, ... )	 name, CAL_PROTO_VALUE_1( __VA_ARGS__ )
#define CAL_PROTO_VALUE_3( role, type, name, ... )	 name, CAL_PROTO_VALUE_2( __VA_ARGS__ )
#define CAL_PROTO_VALUE_4( role, type, name, ... )	 name, CAL_PROTO_VALUE_3( __VA_ARGS__ )
#define CAL_PROTO_VALUE_5( role, type, name, ... )	 name, CAL_PROTO_VALUE_4( __VA_ARGS__ )
#define CAL_PROTO_VALUE_6( role, type, name, ... )	 name, CAL_PROTO_VALUE_5( __VA_ARGS__ )
#define CAL_PROTO_VALUE_7( role, type, name, ... )	 name, CAL_PROTO_VALUE_6( __VA_ARGS__ )
#define CAL_PROTO_VALUE_8( role, type, name, ... )	 name, CAL_PROTO_VALUE_7( __VA_ARGS__ )
#define CAL_PROTO_VALUE_9( role, type, name, ... )	 name, CAL_PROTO_VALUE_8( __VA_ARGS__ )


/* Create list of roles */
#define	CAL_IN		0
#define	CAL_OUT		1
#define	CAL_IO		2
#define	CAL_INVAL	3

#define CAL_PROTO_ROLES( rolesList, numArgs, ... )	\
		rolesList = (char *)malloc(numArgs * sizeof(char));	\
		if (rolesList==NULL) { fprintf(stderr,"CAL Internal error: Allocating task memory for roles list %s[%d]\n", __FILE__, __LINE__ ); exit( 12 ); }	\
		CAL_PROTO_ROLES_##numArgs( rolesList, numArgs, __VA_ARGS__ )

#define CAL_PROTO_ROLES_1( rolesList, numArgs, role, type, name )		rolesList[numArgs - 1] = CAL_##role;
#define CAL_PROTO_ROLES_2( rolesList, numArgs, role, type, name, ... )	rolesList[numArgs - 2] = CAL_##role; CAL_PROTO_ROLES_1( rolesList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_ROLES_3( rolesList, numArgs, role, type, name, ... )	rolesList[numArgs - 3] = CAL_##role; CAL_PROTO_ROLES_2( rolesList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_ROLES_4( rolesList, numArgs, role, type, name, ... )	rolesList[numArgs - 4] = CAL_##role; CAL_PROTO_ROLES_3( rolesList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_ROLES_5( rolesList, numArgs, role, type, name, ... )	rolesList[numArgs - 5] = CAL_##role; CAL_PROTO_ROLES_4( rolesList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_ROLES_6( rolesList, numArgs, role, type, name, ... )	rolesList[numArgs - 6] = CAL_##role; CAL_PROTO_ROLES_5( rolesList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_ROLES_7( rolesList, numArgs, role, type, name, ... )	rolesList[numArgs - 7] = CAL_##role; CAL_PROTO_ROLES_6( rolesList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_ROLES_8( rolesList, numArgs, role, type, name, ... )	rolesList[numArgs - 8] = CAL_##role; CAL_PROTO_ROLES_7( rolesList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_ROLES_9( rolesList, numArgs, role, type, name, ... )	rolesList[numArgs - 9] = CAL_##role; CAL_PROTO_ROLES_8( rolesList, numArgs, __VA_ARGS__ )

/* Create list of pointers */
#define CAL_PROTO_POINTERS( pointersList, numArgs, ... )	\
		pointersList = (void **)malloc(numArgs * sizeof(void *));	\
		if (pointersList==NULL) { printf("CAL Internal error: Allocating task memory for pointers list %s[%d]\n", __FILE__, __LINE__ ); ; }	\
		CAL_PROTO_POINTERS_##numArgs( pointersList, numArgs, __VA_ARGS__ )

#define CAL_PROTO_POINTERS_1( pointersList, numArgs, role, type, name )		pointersList[numArgs - 1] = (void *)(name);
#define CAL_PROTO_POINTERS_2( pointersList, numArgs, role, type, name, ... )	pointersList[numArgs - 2] = (void *)(name); CAL_PROTO_POINTERS_1( pointersList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_POINTERS_3( pointersList, numArgs, role, type, name, ... )	pointersList[numArgs - 3] = (void *)(name); CAL_PROTO_POINTERS_2( pointersList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_POINTERS_4( pointersList, numArgs, role, type, name, ... )	pointersList[numArgs - 4] = (void *)(name); CAL_PROTO_POINTERS_3( pointersList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_POINTERS_5( pointersList, numArgs, role, type, name, ... )	pointersList[numArgs - 5] = (void *)(name); CAL_PROTO_POINTERS_4( pointersList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_POINTERS_6( pointersList, numArgs, role, type, name, ... )	pointersList[numArgs - 6] = (void *)(name); CAL_PROTO_POINTERS_5( pointersList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_POINTERS_7( pointersList, numArgs, role, type, name, ... )	pointersList[numArgs - 7] = (void *)(name); CAL_PROTO_POINTERS_6( pointersList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_POINTERS_8( pointersList, numArgs, role, type, name, ... )	pointersList[numArgs - 8] = (void *)(name); CAL_PROTO_POINTERS_7( pointersList, numArgs, __VA_ARGS__ )
#define CAL_PROTO_POINTERS_9( pointersList, numArgs, role, type, name, ... )	pointersList[numArgs - 9] = (void *)(name); CAL_PROTO_POINTERS_8( pointersList, numArgs, __VA_ARGS__ )

/* Create list of displacement */
#define CAL_PROTO_DISPLACEMENTS( displacementsList, numArgs, ... )	\
		displacementsList = (uint8_t *)malloc((numArgs+1) * sizeof(uint8_t));	\
		if (displacementsList==NULL) { printf("CAL Internal error: Allocating task memory for pointers list %s[%d]\n", __FILE__, __LINE__ ); ; }	\
		displacementsList[0] = 0; \
		CAL_PROTO_DISPLACEMENTS_##numArgs( displacementsList, numArgs, __VA_ARGS__ )

#define CAL_PROTO_DISPLACEMENTS_1( displacementsList, numArgs, role, type, name )		displacementsList[numArgs] = displacementsList[numArgs - 1] + LIST_SIZE_KTILE(role, type, name);    
#define CAL_PROTO_DISPLACEMENTS_2( displacementsList, numArgs, role, type, name, ... )	displacementsList[numArgs - 1] = displacementsList[numArgs - 2] + LIST_SIZE_KTILE(role, type, name); CAL_PROTO_DISPLACEMENTS_1( displacementsList, numArgs, __VA_ARGS__ ); 
#define CAL_PROTO_DISPLACEMENTS_3( displacementsList, numArgs, role, type, name, ... )	displacementsList[numArgs - 2] = displacementsList[numArgs - 3] + LIST_SIZE_KTILE(role, type, name); CAL_PROTO_DISPLACEMENTS_2( displacementsList, numArgs, __VA_ARGS__ ); 
#define CAL_PROTO_DISPLACEMENTS_4( displacementsList, numArgs, role, type, name, ... )	displacementsList[numArgs - 3] = displacementsList[numArgs - 4] + LIST_SIZE_KTILE(role, type, name); CAL_PROTO_DISPLACEMENTS_3( displacementsList, numArgs, __VA_ARGS__ ); 
#define CAL_PROTO_DISPLACEMENTS_5( displacementsList, numArgs, role, type, name, ... )	displacementsList[numArgs - 4] = displacementsList[numArgs - 5] + LIST_SIZE_KTILE(role, type, name); CAL_PROTO_DISPLACEMENTS_4( displacementsList, numArgs, __VA_ARGS__ ); 
#define CAL_PROTO_DISPLACEMENTS_6( displacementsList, numArgs, role, type, name, ... )	displacementsList[numArgs - 5] = displacementsList[numArgs - 6] + LIST_SIZE_KTILE(role, type, name); CAL_PROTO_DISPLACEMENTS_5( displacementsList, numArgs, __VA_ARGS__ ); 
#define CAL_PROTO_DISPLACEMENTS_7( displacementsList, numArgs, role, type, name, ... )	displacementsList[numArgs - 6] = displacementsList[numArgs - 7] + LIST_SIZE_KTILE(role, type, name); CAL_PROTO_DISPLACEMENTS_6( displacementsList, numArgs, __VA_ARGS__ ); 
#define CAL_PROTO_DISPLACEMENTS_8( displacementsList, numArgs, role, type, name, ... )	displacementsList[numArgs - 7] = displacementsList[numArgs - 8] + LIST_SIZE_KTILE(role, type, name); CAL_PROTO_DISPLACEMENTS_7( displacementsList, numArgs, __VA_ARGS__ ); 
#define CAL_PROTO_DISPLACEMENTS_9( displacementsList, numArgs, role, type, name, ... )	displacementsList[numArgs - 8] = displacementsList[numArgs - 9] + LIST_SIZE_KTILE(role, type, name); CAL_PROTO_DISPLACEMENTS_8( displacementsList, numArgs, __VA_ARGS__ ); 

/* Case structure with empty statements to select the implementations that are declared */
#define CAL_KERNEL_IMPL_CASE_1( type, ... )	case type: 
#define CAL_KERNEL_IMPL_CASE_2( type, ... )	case type: CAL_KERNEL_IMPL_CASE_1( __VA_ARGS__ )
#define CAL_KERNEL_IMPL_CASE_3( type, ... )	case type: CAL_KERNEL_IMPL_CASE_2( __VA_ARGS__ )
#define CAL_KERNEL_IMPL_CASE_4( type, ... )	case type: CAL_KERNEL_IMPL_CASE_3( __VA_ARGS__ )
#define CAL_KERNEL_IMPL_CASE_5( type, ... )	case type: CAL_KERNEL_IMPL_CASE_4( __VA_ARGS__ )
#define CAL_KERNEL_IMPL_CASE_6( type, ... )	case type: CAL_KERNEL_IMPL_CASE_5( __VA_ARGS__ )
#define CAL_KERNEL_IMPL_CASE_7( type, ... )	case type: CAL_KERNEL_IMPL_CASE_6( __VA_ARGS__ )

/*
 *******************************************************************************************
 *******************************************************************************************
 ****************************************  KERNEL  *****************************************
 *******************************************************************************************
 *******************************************************************************************
 */


/* @ana 26-07-2017 Adding the index domain to the kernel declaration */
#define CAL_KERNEL_dGPU(name,type, ...)		\
	__device__ void CAL_Kernel_##type##_##name (CALThread threadId, __VA_ARGS__ )

// Use CntrlGPUs' stream member to obtain kernels stream
#define CAL_KERNEL_dGPUCK(name,type, ...)		\
	__device__ void CAL_Kernel_##type##_##name (cudaStream_t handle, CALThread threadId, __VA_ARGS__ )

#define CAL_KERNEL_cuBLAS(name,type, ...)		\
	void CAL_Kernel_##type##_##name (cublasHandle_t handle, CALThread threadId, __VA_ARGS__ )

#define CAL_KERNEL_CPU(name,type, ...)		\
	void CAL_Kernel_##type##_##name (CALThread threadId, __VA_ARGS__ )

#define CAL_KERNEL_XPhi(name,type, ...)		\
	__attribute__((target(mic))) void CAL_Kernel_##type##_##name (CALThread threadId, __VA_ARGS__ )

#define CAL_KERNEL_libXPhi(name,type, ...)		\
	__attribute__((target(mic))) void CAL_Kernel_##type##_##name (CALThread threadId, __VA_ARGS__ )

#define CAL_KERNEL_Generic(name,type, ...)		\
	__host__ __device__ void CAL_Kernel_##type##_##name (CALThread threadId, __VA_ARGS__ )



#ifdef  __CUDACC__
	#define CUDA_CHECK() { cudaError_t error; if ((error = cudaGetLastError())!=cudaSuccess){ fprintf(stdout,"%s: %s\n", cudaGetErrorName(error),cudaGetErrorString(error));exit(EXIT_FAILURE); } }

	#define CAL_KERNEL( name, type, ...) 	\
		 /*__device__ void CAL_Kernel_##type##_##name (CALThread threadId, __VA_ARGS__ )*/ \
		CAL_KERNEL_##type (name,type, __VA_ARGS__)		
#else
	#ifdef __ICC
		#define CAL_KERNEL( name, type, ...)    \
		 	__attribute__((target(mic))) void CAL_Kernel_##type##_##name (CALThread threadId, __VA_ARGS__ ) 
	#else

		#define CAL_KERNEL( name, type, ...) 	\
			void CAL_Kernel_##type##_##name (CALThread threadId, __VA_ARGS__ )		
	#endif
#endif	

/*
 *******************************************************************************************
 *******************************************************************************************
 ************************************  WRAPPERS  *******************************************
 *******************************************************************************************
 *******************************************************************************************
 */

/* @ana 06-10-2017 Macros to define the specific devices wrappers */
#ifdef __CUDACC__

	#define wrapdGPU( name, argsList,type, ...) \
	{   	dim3 grid =  CAL_GPU_kchar_grid(name,CAL_ARCH_KEPLER, threadId); \
                        dim3 block =CAL_GPU_kchar_threads(name,CAL_ARCH_KEPLER);       \
                        launch_dGPU_##name<<<grid, block, 0, handle>>>(threadId,  CAL_ARG_LIST_ACCESS_KTILE( argsList, __VA_ARGS__ ) ); \
	}
//printf("-> Hilos GPU: x %d; y %d\n", block.x, block.y);	                      \	
//printf("grid  %d  %d %d     block  %d  %d  %d  \n", grid.x, grid.y, grid.z, block.x, block.y, block.z); fflush(stdout); \ 
//	                        printf("->->->->->-> Hilos GPU: x %d; y %d    grid %d  %d\n", block.x, block.y,grid.x, grid.y);\

#else
	 #define wrapdGPU( name, ...)	printf("Error, no NVCC found\n");
#endif


/*#define wrapdGPUCK( name, argsList,type, ...) \
{							\
	printf("Here, it is the cuBLAS launcher\n");	\
	cudaStream_t handle_t = *(cudaStream_t*) handle;									\
  	CAL_Kernel_dGPUCK_##name( handle_t, threadId, CAL_ARG_LIST_ACCESS_KTILE( argsList, __VA_ARGS__ ));		\
		 \
}*/	




#define wrapcuBLAS( name, argsList,type, ...) \
{							\
	printf("Here, it is the cuBLAS launcher\n");	\
	//cublasHandle_t handle_t = *(cublasHandle_t*) handle;									\
  	CAL_Kernel_cuBLAS_##name( handle_t, threadId, CAL_ARG_LIST_ACCESS_KTILE( argsList, __VA_ARGS__ ));		\
		// \
}	

#define wrapCPU( name, argsList,type, ...) \
{		\
  CAL_Kernel_CPU_##name(threadId, CAL_ARG_LIST_ACCESS_KTILE( argsList, __VA_ARGS__ ));		\
}	

#ifdef __CUDACC__

	#define wrapGeneric( name, argsList,type, ...) \
	{							\
	if(cntrlS==dGPU){							\
		 dim3 grid = CAL_GPU_kchar_grid(name,CAL_ARCH_KEPLER, threadId); \
	         dim3 block = CAL_GPU_kchar_threads(name,CAL_ARCH_KEPLER);        \
	         launch_Generic_##name<<<grid, block>>>(threadId,  CAL_ARG_LIST_ACCESS_KTILE( argsList, __VA_ARGS__ ) ); \
		 CUDA_CHECK();							\
		}						\
	if(cntrlS==CPU){						\
	  	CAL_Kernel_Generic_##name(threadId, CAL_ARG_LIST_ACCESS_KTILE( argsList, __VA_ARGS__ ));		\
		}						\
	}
#else
	#ifdef __ICC
		#define wrapGeneric( name, argsList,type, ...) \
		{							\
		if(cntrlS==dGPU){							\
			}						\
		if(cntrlS==CPU){						\
		  	CAL_Kernel_Generic_##name(threadId, CAL_ARG_LIST_ACCESS_KTILE( argsList, __VA_ARGS__ ));		\
			}						\
		if(cntrlS ==XPhi){					\
			launch_Generic_Xphi_##name(threadId,  CAL_ARG_LIST_ACCESS_KTILE( argsList, __VA_ARGS__ ));		\
		}					\
		}
	#else
		#define wrapGeneric( name, argsList,type, ...) \
		{							\
		if(cntrlS==dGPU){							\
			}						\
		if(cntrlS==CPU){						\
		  	CAL_Kernel_Generic_##name(threadId, CAL_ARG_LIST_ACCESS_KTILE( argsList, __VA_ARGS__ ));		\
			}						\
		if(cntrlS ==XPhi){					\
		}					\
		}

	#endif
#endif

#ifdef __ICC
	#define wrapXPhi( name, argsList,type, ...) \
		launch_Xphi_##name(threadId,  CAL_ARG_LIST_ACCESS_KTILE( argsList, __VA_ARGS__ )); 

#else
	#define wrapXPhi( name, argsList,type, ...) 
#endif

/* Case structure to call/launch one of the kernel implementations */
#define CAL_KERNEL_WRAP_LAUNCH_1( name, argsList,type, ... )	\
	case CAL_IMPL_##type: wrap##type(name, argsList,type, __VA_ARGS__ ) \
	break; 

#define CAL_KERNEL_WRAP_LAUNCH_2( name, argsList, type, ... )	\
	case CAL_IMPL_##type: wrap##type(name, argsList,type, CAL_PROTO_SKIP_IMPL( PROTO_ROLED, NULL, 1, __VA_ARGS__) ) \
	break;												\
	 CAL_KERNEL_WRAP_LAUNCH_1( name, argsList, __VA_ARGS__ )

#define CAL_KERNEL_WRAP_LAUNCH_3( name, argsList, type, ... )	\
	case CAL_IMPL_##type: wrap##type(name, argsList,type, CAL_PROTO_SKIP_IMPL( PROTO_ROLED, NULL, 2, __VA_ARGS__) ) \
	break;												\
	 CAL_KERNEL_WRAP_LAUNCH_2( name, argsList, __VA_ARGS__ )

#define CAL_KERNEL_WRAP_LAUNCH_4( name, argsList, type, ... )	\
	case CAL_IMPL_##type: wrap##type(name, argsList,type, CAL_PROTO_SKIP_IMPL( PROTO_ROLED, NULL, 3, __VA_ARGS__) ) \
	break;												\
	 CAL_KERNEL_WRAP_LAUNCH_3( name, argsList, __VA_ARGS__ )

#define CAL_KERNEL_WRAP_LAUNCH_5( name, argsList, type, ... )	\
	case CAL_IMPL_##type: wrap##type(name, argsList,type, CAL_PROTO_SKIP_IMPL( PROTO_ROLED, NULL, 4, __VA_ARGS__) ) ;\
	break;												\
	 CAL_KERNEL_WRAP_LAUNCH_4( name, argsList, __VA_ARGS__ )

#define CAL_KERNEL_WRAP_LAUNCH_6( name, argsList, type, ... )	\
	case CAL_IMPL_##type: wrap##type(name, argsList,type, CAL_PROTO_SKIP_IMPL( PROTO_ROLED, NULL, 5, __VA_ARGS__) ) ;\
	break;												\
	 CAL_KERNEL_WRAP_LAUNCH_5( name, argsList, __VA_ARGS__ )

#define CAL_KERNEL_WRAP_LAUNCH_7( name, argsList, type, ... )	\
	case CAL_IMPL_##type: wrap##type(name, argsList,type, CAL_PROTO_SKIP_IMPL( PROTO_ROLED, NULL, 6, __VA_ARGS__) ) ;\
	break;												\
	 CAL_KERNEL_WRAP_LAUNCH_6( name, argsList, __VA_ARGS__ )


/*
 *******************************************************************************************
 *******************************************************************************************
 ************************************  LAUNCHER  *******************************************
 *******************************************************************************************
 *******************************************************************************************
 */

/* @ana 06-10-2017 Macro to generate the kernel launcher for GPUs */
#ifdef  __CUDACC__
	#define launch_dGPU(name,  ...)				\
		  __global__  void launch_dGPU_##name(CALThread threads, CAL_PROTO_TYPED_KTILE( NULL, __VA_ARGS__ )  ){            \			  
			CALThread threadId = { threads.dims, \
                                                threadIdx.x + blockDim.x * blockIdx.x,  \
                                                threadIdx.y + blockDim.y * blockIdx.y, \
                                                threadIdx.z + blockDim.z * blockIdx.z }; \
                        if ( threadId.x >= threads.x  || \
                                threadId.y >= threads.y || \
                                threadId.z >= threads.z ) return; \
                        CAL_Kernel_dGPU_##name(threadId, CAL_PROTO_VALUE( NULL, __VA_ARGS__ ) ); \
                }
#else
	#define launch_dGPU(name, ...)  void launch_dGPU_##name(CALThread threads, CAL_PROTO_TYPED_KTILE( NULL, __VA_ARGS__ ) ){} 

#endif

#ifndef launch_dGPU
	#define launch_dGPUCK(name,  ...)  launch_dGPU(name,  __VA_ARGS__)  
#else
	#define launch_dGPUCK(name,  ...) 
#endif

/* @ana 17-10-2017 Macro to launch Generic Kernels */
#ifdef  __CUDACC__
	#define launch_Generic(name,  ...)  			\
		  __global__  void launch_Generic_##name(CALThread threads, CAL_PROTO_TYPED_KTILE( NULL, __VA_ARGS__ )  ){            \
			CALThread threadId = { threads.dims, \
                                                threadIdx.x + blockDim.x * blockIdx.x,  \
                                                threadIdx.y + blockDim.y * blockIdx.y, \
                                                threadIdx.z + blockDim.z * blockIdx.z }; \
                        if ( threadId.x >= threads.x  || \
                                threadId.y >= threads.y || \
                                threadId.z >= threads.z ) return; \
                        CAL_Kernel_Generic_##name(threadId, CAL_PROTO_VALUE( NULL, __VA_ARGS__ ) ); \
                }
		  
#else
	#ifdef __ICC
	 #define launch_Generic(name,  ...)			\
            void launch_Generic_Xphi_##name(CALThread threads, CAL_PROTO_TYPED_KTILE( NULL, __VA_ARGS__ )  ){                   \
                CAL_PROTO_CAST_IN(argsList, __VA_ARGS__)                                                        \
                _Pragma(CAL_PROTO_OFFLOAD_KTILE(argsList, __VA_ARGS__))   \
                {                               \
                CAL_PROTO_CAST_OUT(argsList, __VA_ARGS__)                       \
                _Pragma("omp parallel") \
                { \
                double Clock = omp_get_wtime() ;\
                { \
                int i,j,k; \
                CALThread threadId; \
                if (threads.z > 1){ \
                        _Pragma("omp for private(i,j,k)") \
                         for(i=0; i<threads.x; i++){  \
                                 for(j=0; j<threads.y; j++){           \
                                        for(k=0; k<threadId.z; k++){            \
                                                threadId.x = i; \
                                                threadId.y = j; \
                                                threadId.z = k; \
                                                CAL_Kernel_Generic_##name(threadId, CAL_PROTO_VALUE( argsList, __VA_ARGS__ ));     \
                        } } }\
                } \
                else if (threads.y > 1){\
                        _Pragma("omp for private(i,j)") \
                         for(i=0; i<threads.x; i++){  \
                                 for(j=0; j<threads.y; j++){           \
                                                threadId.x = i; \
                                                threadId.y = j; \
                                                threadId.z = 0; \
                                                CAL_Kernel_Generic_##name(threadId, CAL_PROTO_VALUE( argsList, __VA_ARGS__ ));     \
                        } } \
                }\
                else {\
                        _Pragma("omp for private(i)") \
                         for(i=0; i<threads.x; i++){  \
                                                threadId.x = i; \
                                                threadId.y = 0; \
                                                threadId.z = 0; \
                                                CAL_Kernel_Generic_##name(threadId, CAL_PROTO_VALUE( argsList, __VA_ARGS__ ));     \
                        }  \
                }\
                Clock = omp_get_wtime() - Clock; \
                } \
               }  \
         }}

	#else
		#define launch_Generic(name,  ...)    
	#endif
#endif


#define launch_CPU(name,  ...) 
	
/* @ana 17-10-2017 Macro to launch Xphi Kernels */
/*
 * Function to group threads into coarse-grained OpenMP tasks
 * Params:
 *      run: CAL_Task object with the info about the kernel to be executed
 *      num_cpus: number of threads associated to the controller object
 *      n_th: task identifier
 */
#define launch_XPhi(name,  ...)			\
            void launch_Xphi_##name(CALThread threads, CAL_PROTO_TYPED_KTILE( NULL, __VA_ARGS__ )  ){ 			\
		CAL_PROTO_CAST_IN(argsList, __VA_ARGS__)							\
		_Pragma(CAL_PROTO_OFFLOAD_KTILE(argsList, __VA_ARGS__))   \
		{				\
		CAL_PROTO_CAST_OUT(argsList, __VA_ARGS__)			\
		_Pragma("omp parallel") \
                { \
                double Clock = omp_get_wtime() ;\
                { \
                int i,j,k; \
                CALThread threadId; \
                if (threads.z > 1){ \
                        _Pragma("omp for private(i,j,k)") \
                         for(i=0; i<threads.x; i++){  \
                                 for(j=0; j<threads.y; j++){           \
                                        for(k=0; k<threadId.z; k++){            \
                                                threadId.x = i; \
                                                threadId.y = j; \
                                                threadId.z = k; \
	  					CAL_Kernel_XPhi_##name(threadId, CAL_PROTO_VALUE( argsList, __VA_ARGS__ ));	\
                        } } }\
                } \
                else if (threads.y > 1){\
                        _Pragma("omp for private(i,j)") \
                         for(i=0; i<threads.x; i++){  \
                                 for(j=0; j<threads.y; j++){           \
                                                threadId.x = i; \
                                                threadId.y = j; \
                                                threadId.z = 0; \
	  					CAL_Kernel_XPhi_##name(threadId, CAL_PROTO_VALUE( argsList, __VA_ARGS__ ));	\
                        } } \
                }\
                else {\
                        _Pragma("omp for private(i)") \
                         for(i=0; i<threads.x; i++){  \
                                                threadId.x = i; \
                                                threadId.y = 0; \
                                                threadId.z = 0; \
	  					CAL_Kernel_XPhi_##name(threadId, CAL_PROTO_VALUE( argsList, __VA_ARGS__ ));	\
                        }  \
                }\
                Clock = omp_get_wtime() - Clock; \
                } \
               }  \
	 }}

#define launch_libXPHI(name,  ...) 


/*
 * Macros to define the functions to launch kernels
 */
#define CAL_KERNEL_LAUNCHER_1(name, type, ...)	\
		launch_##type (name, __VA_ARGS__)

#define CAL_KERNEL_LAUNCHER_2(name, type, ...)	\
		launch_##type (name, CAL_PROTO_SKIP_IMPL( PROTO_ROLED, NULL, 1, __VA_ARGS__))	\
		CAL_KERNEL_LAUNCHER_1(name, __VA_ARGS__) 

#define CAL_KERNEL_LAUNCHER_3(name, type, ...)	\
		launch_##type (name, CAL_PROTO_SKIP_IMPL( PROTO_ROLED, NULL, 2, __VA_ARGS__))	\
		CAL_KERNEL_LAUNCHER_2(name, __VA_ARGS__) 

#define CAL_KERNEL_LAUNCHER_4(name, type, ...)	\
		launch_##type (name, CAL_PROTO_SKIP_IMPL( PROTO_ROLED, NULL, 3, __VA_ARGS__))	\
		CAL_KERNEL_LAUNCHER_3(name, __VA_ARGS__) 

#define CAL_KERNEL_LAUNCHER_5(name, type, ...)	\
		launch_##type (name, CAL_PROTO_SKIP_IMPL( PROTO_ROLED, NULL, 4, __VA_ARGS__))	\
		CAL_KERNEL_LAUNCHER_4(name, __VA_ARGS__) 

#define CAL_KERNEL_LAUNCHER_6(name, type, ...)	\
		launch_##type (name, CAL_PROTO_SKIP_IMPL( PROTO_ROLED, NULL, 5, __VA_ARGS__))	\
		CAL_KERNEL_LAUNCHER_5(name, __VA_ARGS__) 

#define CAL_KERNEL_LAUNCHER_7(name, type, ...)	\
		launch_##type (name, CAL_PROTO_SKIP_IMPL( PROTO_ROLED, NULL, 6, __VA_ARGS__))	\
		CAL_KERNEL_LAUNCHER_6(name, __VA_ARGS__) 
/*
 *******************************************************************************************
 *******************************************************************************************
 ***************************************  MASKs   ******************************************
 *******************************************************************************************
 *******************************************************************************************
 */

#define CAL_KERNEL_MASK_1(type, ...)	\
	CAL_IMPL_##type

#define CAL_KERNEL_MASK_2(type, ...)	\
	CAL_IMPL_##type + \
	CAL_KERNEL_MASK_1( __VA_ARGS__)

#define CAL_KERNEL_MASK_3(type, ...)	\
	type + CAL_KERNEL_MASK_2( __VA_ARGS__)

#define CAL_KERNEL_MASK_4(type, ...)	\
	type + CAL_KERNEL_MASK_3( __VA_ARGS__)

#define CAL_KERNEL_MASK_5(type, ...)	\
	type + CAL_KERNEL_MASK_4( __VA_ARGS__)

#define CAL_KERNEL_MASK_6(type, ...)	\
	type + CAL_KERNEL_MASK_5( __VA_ARGS__)

#define CAL_KERNEL_MASK_7(type, ...)	\
	type + CAL_KERNEL_MASK_6( __VA_ARGS__)

/*
 *******************************************************************************************
 *******************************************************************************************
 ***************************************  PROTO  *******************************************
 *******************************************************************************************
 *******************************************************************************************
 */

/* 
 * Prototype and implementations declaration. 
 * Build the kernel task creation, and the launch wrapper function
 */ 
#define CAL_KERNEL_PROTO( name, numImplementations, ... )	\
		int wrapper_##name(cudaStream_t  handle, CAL_Impl cntrlS, int deviceType, CALThread threadId, void * argsList);\
		CAL_KERNEL_LAUNCHER_##numImplementations(name, __VA_ARGS__)				\
		int wrapper_##name(cudaStream_t  handle, CAL_Impl cntrlS, int deviceType, CALThread threadId, void * argsList ){				\		
			switch ( deviceType ){					\				
				CAL_KERNEL_WRAP_LAUNCH_##numImplementations( name, argsList,__VA_ARGS__ )				\
				default:        \
					fprintf(stderr, "CAL Internal error: Wrong implementation type on launching wrapper: %s, %s[%d]\n", #name, __FILE__, __LINE__ );\
			}\
			return 5;\
		}\
		CAL_Task CAL_kernelTaskCreate_##name( CAL_Impl deviceType, CALThread threads, CAL_PROTO_SKIP_IMPL( PROTO_TYPED_POINTER, NULL, numImplementations, __VA_ARGS__ ) ) { \
			CAL_Task newTask = CAL_TASK_NULL; \
			newTask.threads = threads; \
			newTask.label=CAL_TASK_TYPE_KERNEL; \
			newTask.narguments= CAL_PROTO_SKIP_IMPL(ARG_COUNT_LIST_ELEMENTS, NULL, numImplementations, __VA_ARGS__); \
			newTask.kernelWrapper = wrapper_##name;\
			CAL_PROTO_SKIP_IMPL( ARG_LIST_CREATE_KTILE, newTask.arguments, numImplementations, __VA_ARGS__);	\
			CAL_PROTO_SKIP_IMPL( PROTO_ROLES, newTask.roles, numImplementations, __VA_ARGS__);	\
			CAL_PROTO_SKIP_IMPL( PROTO_POINTERS, newTask.pointers, numImplementations, __VA_ARGS__);	\
			CAL_PROTO_SKIP_IMPL( PROTO_DISPLACEMENTS, newTask.displacements, numImplementations, __VA_ARGS__);	\
			int numImpls= CAL_KERNEL_MASK_##numImplementations(__VA_ARGS__); 	\
			int dev = CAL_dev(deviceType, numImpls);		\
			newTask.deviceId = dev;					 \
			return newTask; \
		}\
		CAL_Task CAL_concurrentKernelTaskCreate_##name( CAL_Impl deviceType, CALThread threads, CAL_Request* outrequest, int count, CAL_Request* inrequests,  CAL_PROTO_SKIP_IMPL( PROTO_TYPED_POINTER, NULL, numImplementations, __VA_ARGS__ ) ) { \
			CAL_Task newTask = CAL_TASK_NULL; \
			newTask.threads = threads; \
			newTask.label=CAL_TASK_TYPE_KERNEL; \
			newTask.async=1; \
			newTask.narguments= CAL_PROTO_SKIP_IMPL(ARG_COUNT_LIST_ELEMENTS, NULL, numImplementations, __VA_ARGS__); \
			newTask.kernelWrapper = wrapper_##name;\
			CAL_PROTO_SKIP_IMPL( ARG_LIST_CREATE_KTILE, newTask.arguments, numImplementations, __VA_ARGS__);	\
			CAL_PROTO_SKIP_IMPL( PROTO_ROLES, newTask.roles, numImplementations, __VA_ARGS__);	\
			CAL_PROTO_SKIP_IMPL( PROTO_POINTERS, newTask.pointers, numImplementations, __VA_ARGS__);	\
			CAL_PROTO_SKIP_IMPL( PROTO_DISPLACEMENTS, newTask.displacements, numImplementations, __VA_ARGS__);	\
			int numImpls= CAL_KERNEL_MASK_##numImplementations(__VA_ARGS__); 	\
			int dev = CAL_dev(deviceType, numImpls);		\
			newTask.deviceId = dev;					 \
			newTask.outrequest = outrequest;\
			newTask.ninrequests = count;\
			newTask.inrequests = inrequests;\
			return newTask; \
		}
		
#endif // _CAL_KProto_
