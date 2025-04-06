cmake_minimum_required (VERSION 3.20)

# Common OpenCL options
if (SUPPORT_OPENCL)
	set(CTRL_EXAMPLES_OPENCL_GPU_PROFILING OFF) # Profiling with this
	set(CTRL_EXAMPLES_OPENCL_GPU_GENERATE_VISUAL_PROFILER OFF) # Complete ifo about events
	set(CTRL_EXAMPLES_OPENCL_GPU_LOCAL_SIZE "16") # Localsize
endif (SUPPORT_OPENCL)

if (SUPPORT_FPGA)
	set(CTRL_EXAMPLES_FPGA_LOCAL_SIZE "16") # Block size
    set(CTRL_EXAMPLES_FPGA_EXTRA_OPTS -ffp-reassociate -ffp-contract=fast -no-interleaving=default)
    set(AOC_CTRL_DEFS ${AOC_CTRL_DEFS} -DBLOCK_SIZE=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE})
endif (SUPPORT_FPGA)

if (SUPPORT_CUDA)
	set(CTRL_EXAMPLES_CUDA_BLOCKSIZE "16") # Block Size
endif(SUPPORT_CUDA)

if (SUPPORT_HIP)
	set(CTRL_EXAMPLES_HIP_BLOCKSIZE "16") # Block Size
endif(SUPPORT_HIP)

if (SUPPORT_CPU)
	set(CTRL_EXAMPLES_CPU_BLOCKSIZE "16") # Block Size
endif(SUPPORT_CPU)

# Apply options
if (CTRL_EXAMPLES_OPENCL_GPU_PROFILING)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_OPENCL_GPU_PROFILING_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_OPENCL_GPU_PROFILING_ ")
endif (CTRL_EXAMPLES_OPENCL_GPU_PROFILING)

if (CTRL_EXAMPLES_OPENCL_GPU_GENERATE_VISUAL_PROFILER)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_OPENCL_GPU_GENERATE_VISUAL_PROFILER_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_OPENCL_GPU_GENERATE_VISUAL_PROFILER_ ")
endif (CTRL_EXAMPLES_OPENCL_GPU_GENERATE_VISUAL_PROFILER)

if (SUPPORT_OPENCL)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOCAL_SIZE=${CTRL_EXAMPLES_OPENCL_GPU_LOCAL_SIZE} ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -DLOCAL_SIZE=${CTRL_EXAMPLES_OPENCL_GPU_LOCAL_SIZE} ")
endif(SUPPORT_OPENCL)

if (SUPPORT_FPGA)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOCAL_SIZE=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE} ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCAL_SIZE=${CTRL_EXAMPLES_FPGA_LOCAL_SIZE} ")
endif(SUPPORT_FPGA)

if (SUPPORT_CUDA)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBLOCKSIZE=${CTRL_EXAMPLES_CUDA_BLOCKSIZE} ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -DBLOCKSIZE=${CTRL_EXAMPLES_CUDA_BLOCKSIZE} ")
endif(SUPPORT_CUDA)

if (SUPPORT_HIP)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBLOCKSIZE=${CTRL_EXAMPLES_HIP_BLOCKSIZE} ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBLOCKSIZE=${CTRL_EXAMPLES_HIP_BLOCKSIZE} ")
endif(SUPPORT_HIP)

if (SUPPORT_CPU)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBLOCKSIZE=${CTRL_EXAMPLES_CPU_BLOCKSIZE} ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -DBLOCKSIZE=${CTRL_EXAMPLES_CPU_BLOCKSIZE} ")
endif(SUPPORT_CPU)

# Verbose
if (CTRL_CMAKE_VERBOSE_EXAMPLES_MATRIX_POWER_OF)
	message(STATUS "MATRIX_POWER_OF_INCLUDE DIRS = ${CTRL_INCLUDE_DIRS}")
	message(STATUS "MATRIX_POWER_OF_LIBS = ${CTRL_LIBS}")
	message(STATUS "MATRIX_POWER_OF_CTRL_C vars = ${CMAKE_C_FLAGS}")
	message(STATUS "MATRIX_POWER_OF_CTRL_CXX vars = ${CMAKE_CXX_FLAGS}")
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

# Hip
if (SUPPORT_HIP)
	add_executable(Matrix_Power_Of_Hip_Ctrl src/Matrix_Power_Of_Hip_Ctrl.cpp)
	target_link_libraries(Matrix_Power_Of_Hip_Ctrl ${CTRL_LIBS})

	add_executable(Matrix_Power_Of_Hip_Ref_Async src/Matrix_Power_Of_Hip_Ref_Async.cpp)
	target_link_libraries(Matrix_Power_Of_Hip_Ref_Async ${CTRL_LIBS})
	
	add_executable(Matrix_Power_Of_Hip_Ref_Sync src/Matrix_Power_Of_Hip_Ref_Sync.cpp)
	target_link_libraries(Matrix_Power_Of_Hip_Ref_Sync ${CTRL_LIBS})
endif(SUPPORT_HIP)

#FPGA
if (SUPPORT_FPGA)
	add_executable(Matrix_Power_Of_FPGA_Ctrl src/Matrix_Power_Of_FPGA_Ctrl.c)
	target_link_libraries(Matrix_Power_Of_FPGA_Ctrl ${CTRL_LIBS})

    add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/Matrix_Power_Of_Ctrl.aocx
        COMMAND aoc ${AOC_CTRL_DEFS} ${AOC_CTRL_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/src/Matrix_Power_Of_FPGA_Ctrl.cl -o ${CMAKE_CURRENT_SOURCE_DIR}/src/Matrix_Power_Of_Ctrl.aocx ${FPGA_AOC_FLAGS} ${CTRL_EXAMPLES_FPGA_EXTRA_OPTS}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/Matrix_Power_Of_FPGA_Ctrl.cl)
    add_custom_target(Matrix_Power_Of_FPGA_Ctrl_kernel DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/Matrix_Power_Of_Ctrl.aocx ${CMAKE_CURRENT_SOURCE_DIR}/src/Matrix_Power_Of_FPGA_Ctrl.cl)
    add_dependencies(Matrix_Power_Of_FPGA_Ctrl Matrix_Power_Of_FPGA_Ctrl_kernel)

	add_executable(Matrix_Power_Of_FPGA_Ref_Async src/Matrix_Power_Of_FPGA_Ref_Async.c)
	add_executable(Matrix_Power_Of_FPGA_Ref_Sync src/Matrix_Power_Of_FPGA_Ref_Sync.c)
	
	target_link_libraries(Matrix_Power_Of_FPGA_Ref_Async ${CTRL_LIBS})
	target_link_libraries(Matrix_Power_Of_FPGA_Ref_Sync ${CTRL_LIBS})

    add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/Matrix_Power_Of_Ref.aocx
        COMMAND aoc ${AOC_CTRL_DEFS} ${CMAKE_CURRENT_SOURCE_DIR}/src/Matrix_Power_Of_FPGA_Ref.cl -o ${CMAKE_CURRENT_SOURCE_DIR}/src/Matrix_Power_Of_Ref.aocx ${FPGA_AOC_FLAGS} ${CTRL_EXAMPLES_FPGA_EXTRA_OPTS}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/Matrix_Power_Of_FPGA_Ref.cl)
    add_custom_target(Matrix_Power_Of_FPGA_Ref_kernel DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/Matrix_Power_Of_Ref.aocx ${CMAKE_CURRENT_SOURCE_DIR}/src/Matrix_Power_Of_FPGA_Ref.cl)
    add_dependencies(Matrix_Power_Of_FPGA_Ref_Async Matrix_Power_Of_FPGA_Ref_kernel)
    add_dependencies(Matrix_Power_Of_FPGA_Ref_Sync Matrix_Power_Of_FPGA_Ref_kernel)
endif (SUPPORT_FPGA)

# OpenCL
if (SUPPORT_OPENCL)
	add_executable(Matrix_Power_Of_OpenCL_Gpu_Ctrl src/Matrix_Power_Of_OpenCL_Gpu_Ctrl.c)
	target_link_libraries(Matrix_Power_Of_OpenCL_Gpu_Ctrl ${CTRL_LIBS})

	add_executable(Matrix_Power_Of_OpenCL_Gpu_Ref_Async src/Matrix_Power_Of_OpenCL_Gpu_Ref_Async.c)
	target_link_libraries(Matrix_Power_Of_OpenCL_Gpu_Ref_Async ${CTRL_LIBS})

	add_executable(Matrix_Power_Of_OpenCL_Gpu_Ref_Sync src/Matrix_Power_Of_OpenCL_Gpu_Ref_Sync.c)
	target_link_libraries(Matrix_Power_Of_OpenCL_Gpu_Ref_Sync ${CTRL_LIBS})
endif (SUPPORT_OPENCL)

# Cpu
if (SUPPORT_CPU)
	add_executable(Matrix_Power_Of_Cpu_Ctrl src/Matrix_Power_Of_Cpu_Ctrl.c)
	target_link_libraries(Matrix_Power_Of_Cpu_Ctrl ${CTRL_LIBS})

	add_executable(Matrix_Power_Of_Cpu_Ref_Sync src/Matrix_Power_Of_Cpu_Ref_Sync.c)
	target_link_libraries(Matrix_Power_Of_Cpu_Ref_Sync ${CTRL_LIBS})
endif (SUPPORT_CPU)
