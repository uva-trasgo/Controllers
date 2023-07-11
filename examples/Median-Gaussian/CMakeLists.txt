# <license>
# 
# Controller v2.1
# 
# This software is provided to enhance knowledge and encourage progress in the scientific
# community. It should be used only for research and educational purposes. Any reproduction
# or use for commercial purpose, public redistribution, in source or binary forms, with or 
# without modifications, is NOT ALLOWED without the previous authorization of the copyright 
# holder. The origin of this software must not be misrepresented; you must not claim that you
# wrote the original software. If you use this software for any purpose (e.g. publication),
# a reference to the software package and the authors must be included.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
# THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# Copyright (c) 2007-2020, Trasgo Group, Universidad de Valladolid.
# All rights reserved.
# 
# More information on http://trasgo.infor.uva.es/
# 
# </license>
cmake_minimum_required (VERSION 3.17)

# Common OpenCL options
if (SUPPORT_OPENCL_GPU)
	set(CTRL_EXAMPLES_OPENCL_GPU_PROFILING OFF) # Profiling with this
	set(CTRL_EXAMPLES_OPENCL_GPU_ERROR_CHECK OFF) # Check errors with this
	set(CTRL_EXAMPLES_OPENCL_GPU_DEBUG OFF) # Debug whit this
	set(CTRL_EXAMPLES_OPENCL_GPU_GENERATE_VISUAL_PROFILER OFF) # Complete ifo about events
	set(CTRL_EXAMPLES_OPENCL_GPU_LOCAL_SIZE "16") # Localsize
endif (SUPPORT_OPENCL_GPU)

if (SUPPORT_FPGA)
	set(CTRL_EXAMPLES_FPGA_PROFILING OFF) # Profiling with this
	set(CTRL_EXAMPLES_FPGA_ERROR_CHECK OFF) # Check errors with this
	set(CTRL_EXAMPLES_FPGA_DEBUG OFF) # Debug whit this
	set(CTRL_EXAMPLES_FPGA_TEST_MODE OFF) # Test results style with this
	set(CTRL_EXAMPLES_FPGA_GENERATE_VISUAL_PROFILER OFF) # Complete ifo about events
	set(CTRL_EXAMPLES_FPGA_LOCAL_SIZE_0 "16") # Localsize 0
	set(CTRL_EXAMPLES_FPGA_LOCAL_SIZE_1 "16") # Localsize 1
	set(CTRL_EXAMPLES_FPGA_LOCAL_SIZE_2 "0") # Localsize 0
endif (SUPPORT_FPGA)

if (SUPPORT_PYNQ)
	set(CTRL_EXAMPLES_FPGA_PROFILING OFF) # Profiling with this
	set(CTRL_EXAMPLES_FPGA_ERROR_CHECK OFF) # Check errors with this
	set(CTRL_EXAMPLES_FPGA_DEBUG OFF) # Debug whit this
	set(CTRL_EXAMPLES_FPGA_TEST_MODE OFF) # Test results style with this
	set(CTRL_EXAMPLES_FPGA_GENERATE_VISUAL_PROFILER OFF) # Complete ifo about events
	set(CTRL_EXAMPLES_FPGA_LOCAL_SIZE_0 "16") # Localsize 0
	set(CTRL_EXAMPLES_FPGA_LOCAL_SIZE_1 "16") # Localsize 1
	set(CTRL_EXAMPLES_FPGA_LOCAL_SIZE_2 "0") # Localsize 0

    set(N_RR "2")

    #set(EXPERIMENTATION_MODE ON)
    set(DEBUG_PYNQ OFF)

    message("DEBUG_PYNQ: " ${DEBUG_PYNQ})
endif (SUPPORT_PYNQ)

if (SUPPORT_CUDA)
	set(CTRL_EXAMPLES_CUDA_ERROR_CHECK OFF) # Check errors with this
	set(CTRL_EXAMPLES_CUDA_DEBUG OFF) # Debug whit this
	set(CTRL_EXAMPLES_CUDA_BLOCKSIZE "16") # Block Size
endif(SUPPORT_CUDA)

if (SUPPORT_CPU)
	set(CTRL_EXAMPLES_CPU_BLOCKSIZE "16") # Block Size
endif(SUPPORT_CPU)

# Apply options
if (CTRL_EXAMPLES_OPENCL_GPU_ERROR_CHECK)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_OPENCL_GPU_ERROR_CHECK_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_OPENCL_GPU_ERROR_CHECK_ ")
endif (CTRL_EXAMPLES_OPENCL_GPU_ERROR_CHECK)

if (CTRL_EXAMPLES_OPENCL_GPU_PROFILING)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_OPENCL_GPU_PROFILING_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_OPENCL_GPU_PROFILING_ ")
endif (CTRL_EXAMPLES_OPENCL_GPU_PROFILING)

if (CTRL_EXAMPLES_OPENCL_GPU_DEBUG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_OPENCL_GPU_DEBUG_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_OPENCL_GPU_DEBUG_ ")
endif (CTRL_EXAMPLES_OPENCL_GPU_DEBUG)

if (CTRL_EXAMPLES_OPENCL_GPU_GENERATE_VISUAL_PROFILER)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_OPENCL_GPU_GENERATE_VISUAL_PROFILER_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_OPENCL_GPU_GENERATE_VISUAL_PROFILER_ ")
endif (CTRL_EXAMPLES_OPENCL_GPU_GENERATE_VISUAL_PROFILER)

if (CTRL_EXAMPLES_FPGA_ERROR_CHECK)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_FPGA_ERROR_CHECK_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_FPGA_ERROR_CHECK_ ")
endif (CTRL_EXAMPLES_FPGA_ERROR_CHECK)

if (CTRL_EXAMPLES_FPGA_PROFILING)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_FPGA_PROFILING_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_FPGA_PROFILING_ ")
endif (CTRL_EXAMPLES_FPGA_PROFILING)

if (CTRL_EXAMPLES_FPGA_DEBUG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_FPGA_DEBUG_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_FPGA_DEBUG_ ")
endif (CTRL_EXAMPLES_FPGA_DEBUG)

if (CTRL_EXAMPLES_FPGA_GENERATE_VISUAL_PROFILER)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_FPGA_GENERATE_VISUAL_PROFILER_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_FPGA_GENERATE_VISUAL_PROFILER_ ")
endif (CTRL_EXAMPLES_FPGA_GENERATE_VISUAL_PROFILER)

if (SUPPORT_OPENCL_GPU)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOCAL_SIZE=${CTRL_EXAMPLES_OPENCL_GPU_LOCAL_SIZE} ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -DLOCAL_SIZE=${CTRL_EXAMPLES_OPENCL_GPU_LOCAL_SIZE} ")
endif(SUPPORT_OPENCL_GPU)

if (SUPPORT_FPGA)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOCAL_SIZE_0=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE_0} -DLOCAL_SIZE_1=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE_1} -DLOCAL_SIZE_2=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE_2} -DKERNELTYPE=NDRANGE -DCTRL_KERNEL_PATH=${CTRL_KERNELS_PATH_FPGA} -DREF_KERNEL_PATH=${REF_KERNELS_PATH_FPGA}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCAL_SIZE_0=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE_0} -DLOCAL_SIZE_1=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE_1} -DLOCAL_SIZE_2=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE_2} -DKERNELTYPE=NDRANGE -DCTRL_KERNEL_PATH=${CTRL_KERNELS_PATH_FPGA} -DREF_KERNEL_PATH=${REF_KERNELS_PATH_FPGA}")
endif(SUPPORT_FPGA)

if (SUPPORT_PYNQ)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOCAL_SIZE_0=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE_0} -DLOCAL_SIZE_1=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE_1} -DLOCAL_SIZE_2=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE_2} -D_CTRL_PYNQ_HOST_ -save-temps -DN_RR=${N_RR}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCAL_SIZE_0=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE_0} -DLOCAL_SIZE_1=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE_1} -DLOCAL_SIZE_2=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE_2} -D_CTRL_PYNQ_HOST_ -save-temps")
    set(CTRL_INCLUDE_DIRS ${CTRL_INCLUDE_DIRS} "${PROJECT_SOURCE_DIR}/kernels/Pynq/median-blur")
    set(CTRL_INCLUDE_DIRS ${CTRL_INCLUDE_DIRS} "${PROJECT_SOURCE_DIR}/kernels/Pynq/gaussian-blur")
    set(CTRL_INCLUDE_DIRS ${CTRL_INCLUDE_DIRS} "${PROJECT_SOURCE_DIR}/scheduler")
endif(SUPPORT_PYNQ)

if (CTRL_EXAMPLES_CUDA_ERROR_CHECK)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_CUDA_ERROR_CHECK_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_CUDA_ERROR_CHECK_ ")
endif (CTRL_EXAMPLES_CUDA_ERROR_CHECK)

if (CTRL_EXAMPLES_CUDA_DEBUG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_CUDA_DEBUG_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_CUDA_DEBUG_ ")
endif (CTRL_EXAMPLES_CUDA_DEBUG)

if (SUPPORT_CUDA)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBLOCKSIZE=${CTRL_EXAMPLES_CUDA_BLOCKSIZE} ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -DBLOCKSIZE=${CTRL_EXAMPLES_CUDA_BLOCKSIZE} ")
	set(CUDA_PROPAGATE_HOST_FLAGS ON)
endif(SUPPORT_CUDA)

if (SUPPORT_CPU)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBLOCKSIZE=${CTRL_EXAMPLES_CPU_BLOCKSIZE} ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -DBLOCKSIZE=${CTRL_EXAMPLES_CPU_BLOCKSIZE} ")
endif(SUPPORT_CPU)

# Verbose
if (CTRL_CMAKE_VERBOSE_EXAMPLES_MATRIX_POWER_OF)
	message(STATUS "MATRIX_POWER_OF_INCLUDE DIRS = ${CTRL_INCLUDE_DIRS}")
	message(STATUS "MATRIX_POWER_OF_LIBS = ${CTRL_LIBS}")
	message(STATUS "MATRIX_POWER_OF_CTRL_C vars = ${CMAKE_C_FLAGS}")
endif(CTRL_CMAKE_VERBOSE_EXAMPLES_MATRIX_POWER_OF)

# Include directories
include_directories(${CTRL_INCLUDE_DIRS})

# Cuda
if (SUPPORT_CUDA)
	add_executable(Matrix_Power_Of_Cuda_Ctrl src/Matrix_Power_Of_Cuda_Ctrl.cu)
	target_link_libraries(Matrix_Power_Of_Cuda_Ctrl ${CTRL_LIBS})

	add_executable(Matrix_Power_Of_Cuda_Ref_Async src/Matrix_Power_Of_Cuda_Ref_Async.cu)
	target_link_libraries(Matrix_Power_Of_Cuda_Ref_Async ${CTRL_LIBS})
	
	add_executable(Matrix_Power_Of_Cuda_Ref_Sync src/Matrix_Power_Of_Cuda_Ref_Sync.cu)
	target_link_libraries(Matrix_Power_Of_Cuda_Ref_Sync ${CTRL_LIBS})
endif(SUPPORT_CUDA)

#FPGA
if (SUPPORT_FPGA)
	if(COMPILE_KERNELS)
		option(COMPILE_KERNEL_MULT "COMPILE_KERNEL_MULT" ON)
	else(COMPILE_KERNELS)
		option(COMPILE_KERNEL_MULT "COMPILE_KERNEL_MULT" OFF)
	endif(COMPILE_KERNELS)
	
	execute_process(COMMAND ${CMAKE_C_COMPILER} ${KERNELS_PATH}/FPGA/parser_ctrlkernel.c -o ${KERNELS_PATH}/FPGA/parser_ctrlkernel)

	execute_process(COMMAND ${KERNELS_PATH}/FPGA/parser_ctrlkernel host ${CTRL_KERNELS_PATH_FPGA}/Mult/Mult_Ctrl.cl ${CTRL_KERNELS_PATH_FPGA}/Mult/Mult_Ctrl.c)

	execute_process(COMMAND rm ${KERNELS_PATH}/FPGA/parser_ctrlkernel)

	include_directories(src/ ${KERNELS_PATH})
	add_executable(Matrix_Pow_FPGA_Ctrl ${CTRL_KERNELS_PATH_FPGA}/Mult/Mult_Ctrl.c src/Matrix_Power_Of_FPGA_Ctrl.c)
	target_link_libraries(Matrix_Pow_FPGA_Ctrl ${CTRL_LIBS})

	add_executable(Matrix_Pow_FPGA_Ref_Async src/Matrix_Power_Of_FPGA_Ref_Async.c)
	add_executable(Matrix_Pow_FPGA_Ref_Sync src/Matrix_Power_Of_FPGA_Ref_Sync.c)
	

	target_link_libraries(Matrix_Pow_FPGA_Ref_Async ${CTRL_LIBS})
	target_link_libraries(Matrix_Pow_FPGA_Ref_Sync ${CTRL_LIBS})
endif (SUPPORT_FPGA)

if (SUPPORT_PYNQ)
    option(DEBUG_OUTPUT "DEBUG_OUTPUT" ON)
    option(DEBUG_PARTIAL "DEBUG_PARTIAL" OFF)
    option(EXTERNAL_IMG "EXTERNAL_IMG" OFF)
    option(TEST_PARTIAL "TEST_PARTIAL" OFF)
    option(TEST_PARTIAL_GAUSSIANBLUR "TEST_PARTIAL_GAUSSIANBLUR" OFF)
    option(TEST_PARTIAL_MEDIANBLUR "TEST_PARTIAL_MEDIANBLUR" OFF)

    message("TEST_PARTIAL: " ${TEST_PARTIAL})

    if (DEBUG_OUTPUT)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DEBUG_OUTPUTS")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG_OUTPUTS")
    endif (DEBUG_OUTPUT)

    if (DEBUG_PARTIAL)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DEBUG_PARTIAL")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG_PARTIAL")
    endif (DEBUG_PARTIAL)

    if(EXPERIMENTATION_MODE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_EXP_MODE")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_EXP_MODE")
    endif(EXPERIMENTATION_MODE)

    message("DEBUG_PYNQ: " ${DEBUG_PYNQ})
    if(DEBUG_PYNQ)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DEBUG")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")
    endif(DEBUG_PYNQ)

    if(EXTERNAL_IMG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_EXTERNAL_IMG")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_EXTERNAL_IMG")
    endif(EXTERNAL_IMG)

    if(TEST_PARTIAL)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_TEST_PARTIAL")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_TEST_PARTIAL")
    endif(TEST_PARTIAL)
    
    if(TEST_PARTIAL_GAUSSIANBLUR)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_TEST_PARTIAL_GAUSSIANBLUR")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_TEST_PARTIAL_GAUSSIANBLUR")
    endif(TEST_PARTIAL_GAUSSIANBLUR)

    if(TEST_PARTIAL_MEDIANBLUR)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_TEST_PARTIAL_MEDIANBLUR")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_TEST_PARTIAL_MEDIANBLUR")
    endif(TEST_PARTIAL_MEDIANBLUR)

    message("CMAKE_C_FLAGS: " ${CMAKE_C_FLAGS})

    include_directories(/usr/include/python3.6m)

    add_executable(Median-Gaussian_Pynq_Ctrl src/Median-Gaussian_Pynq_Ctrl.c ${PROJECT_SOURCE_DIR}/kernels/Pynq/median-blur/median-blur.c ${PROJECT_SOURCE_DIR}/kernels/Pynq/gaussian-blur/gaussian-blur.c ${PROJECT_SOURCE_DIR}/scheduler/task_generator.c ${PROJECT_SOURCE_DIR}/scheduler/util.c ${PROJECT_SOURCE_DIR}/scheduler/task_queue.c ${PROJECT_SOURCE_DIR}/scheduler/run.c ${PROJECT_SOURCE_DIR}/scheduler/scheduler.c ${PROJECT_SOURCE_DIR}/extern/SyncRNG/python/src/_syncrng.c)
    #add_executable(Median-Gaussian_Pynq_Ctrl src/Median-Gaussian_Pynq_Ctrl.c ${PROJECT_SOURCE_DIR}/kernels/Pynq/median-blur/median-blur.c ${PROJECT_SOURCE_DIR}/kernels/Pynq/gaussian-blur/gaussian-blur.c src/task_generator.c src/no_preemption_scheduler.c src/task_generator.c src/util.c)
    set(CTRL_LIBS ${CTRL_LIBS} "-lpynq -lcma")
    message(STATUS "-----> CTRL_LIBS: ${CTRL_LIBS}")
    target_link_libraries(Median-Gaussian_Pynq_Ctrl ${CTRL_LIBS} -lpython3.6m)

    execute_process(COMMAND ${CMAKE_C_COMPILER} ${PROJECT_SOURCE_DIR}/scheduler/parser.c -o ${PROJECT_SOURCE_DIR}/scheduler/parser)
    execute_process(COMMAND ${PROJECT_SOURCE_DIR}/scheduler/parser ${PROJECT_SOURCE_DIR}/scheduler/kernels.h OUTPUT_FILE ${PROJECT_SOURCE_DIR}/scheduler/kernel_names.txt)
    execute_process(COMMAND ${PROJECT_SOURCE_DIR}/scheduler/parser ${PROJECT_SOURCE_DIR}/scheduler/kernels.h OUTPUT_VARIABLE KERNEL_NAMES_NEWLINE
                    COMMAND sed -zE 's/\n/;/g' ${KERNEL_NAMES_NEWLINE} OUTPUT_VARIABLE KERNEL_NAMES)
    message("==== BEFORE SED")

    message("===== BEFORE KERNEL NAMES")
    foreach( name IN LISTS ${KERNEL_NAMES} )
        message("name: " ${name})
    endforeach()
endif (SUPPORT_PYNQ)

# OpenCl
if (SUPPORT_OPENCL_GPU)
	add_executable(Matrix_Power_Of_OpenCL_Gpu_Ctrl src/Matrix_Power_Of_OpenCL_Gpu_Ctrl.c)
	target_link_libraries(Matrix_Power_Of_OpenCL_Gpu_Ctrl ${CTRL_LIBS})

	add_executable(Matrix_Power_Of_OpenCL_Gpu_Ref_Async src/Matrix_Power_Of_OpenCL_Gpu_Ref_Async.c)
	target_link_libraries(Matrix_Power_Of_OpenCL_Gpu_Ref_Async ${CTRL_LIBS})

	add_executable(Matrix_Power_Of_OpenCL_Gpu_Ref_Sync src/Matrix_Power_Of_OpenCL_Gpu_Ref_Sync.c)
	target_link_libraries(Matrix_Power_Of_OpenCL_Gpu_Ref_Sync ${CTRL_LIBS})
endif (SUPPORT_OPENCL_GPU)

# Cpu
if (SUPPORT_CPU)
	add_executable(Matrix_Power_Of_Cpu_Ctrl src/Matrix_Power_Of_Cpu_Ctrl.c)
	target_link_libraries(Matrix_Power_Of_Cpu_Ctrl ${CTRL_LIBS})

	add_executable(Matrix_Power_Of_Cpu_Ref_Sync src/Matrix_Power_Of_Cpu_Ref_Sync.c)
	target_link_libraries(Matrix_Power_Of_Cpu_Ref_Sync ${CTRL_LIBS})
endif (SUPPORT_CPU)
