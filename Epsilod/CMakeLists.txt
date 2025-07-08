cmake_minimum_required(VERSION 3.20)

# Apply options
if(CTRL_EXAMPLES_CUDA_ERROR_CHECK)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_CUDA_ERROR_CHECK_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_CUDA_ERROR_CHECK_ ")
endif(CTRL_EXAMPLES_CUDA_ERROR_CHECK)

# Verbose
if(CTRL_CMAKE_VERBOSE_EXAMPLES)
	message(STATUS "PARALLEL_STENCIL_SKELETON_INCLUDE DIRS = ${CTRL_INCLUDE_DIRS}")
	message(STATUS "PARALLEL_STENCIL_SKELETON_LIBS = ${CTRL_LIBS}")
	message(STATUS "PARALLEL_STENCIL_SKELETON_CTRL_C vars = ${CMAKE_C_FLAGS}")
	message(STATUS "PARALLEL_STENCIL_SKELETON_CTRL_CXX vars = ${CMAKE_CXX_FLAGS}")
	message(STATUS "PARALLEL_STENCIL_SKELETON_CTRL_CUDA vars = ${CMAKE_CUDA_FLAGS}")
endif(CTRL_CMAKE_VERBOSE_EXAMPLES)

# Include directories and libs
include_directories(${CTRL_INCLUDE_DIRS} src/)
set(CTRL_LIBS ${CTRL_LIBS} Ctrl)

# Compile kernels with nvcc if CUDA is active. If not, compile them with the C compiler
if(SUPPORT_CUDA)
	set(EPSILOD_KERNELS "src/epsilod_kernels.cu")
	set(TEST_KERNEL_SOURCE "src/test_parallelStencilSkeleton_kernels.cu")
elseif(SUPPORT_HIP)
	set(EPSILOD_KERNELS "src/epsilod_kernels.cpp")
	set(TEST_KERNEL_SOURCE "src/test_parallelStencilSkeleton_kernels.cpp")
else(SUPPORT_CUDA)
	set(EPSILOD_KERNELS "src/epsilod_kernels.c")
	set(TEST_KERNEL_SOURCE "src/test_parallelStencilSkeleton_kernels.c")
endif(SUPPORT_CUDA)

add_executable(
	test_parallelStencilSkeleton
	${EPSILOD_KERNELS}
	${TEST_KERNEL_SOURCE}
	src/test_parallelStencilSkeleton.c
	src/epsilod.c
)
target_link_libraries(test_parallelStencilSkeleton ${CTRL_LIBS})

# Compile FPGA kernels
if(SUPPORT_FPGA)
	file(GLOB epsilod_fpga_kernel "src/fpga_kernels/*.cl")
	foreach(kernel_src_path ${epsilod_fpga_kernel})
		cmake_path(GET kernel_src_path STEM kernel_name) # Remove extension

		add_custom_command(
			OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/fpga_kernels/${kernel_name}.aocx
			COMMAND
				time aoc -g -v -report -ffp-reassociate -ffp-contract=fast -no-hardware-kernel-invocation-queue -opt-arg=-nocaching
				${AOC_CTRL_DEFS} ${AOC_CTRL_INCLUDES} ${kernel_src_path} -o
				${CMAKE_CURRENT_SOURCE_DIR}/src/fpga_kernels/${kernel_name}.aocx ${FPGA_AOC_FLAGS}
			DEPENDS ${kernel_src_path}
		)
		add_custom_target(
			epsilod_fpga_kernel_${kernel_name}
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/fpga_kernels/${kernel_name}.aocx ${kernel_src_path}
		)
		add_dependencies(test_parallelStencilSkeleton epsilod_fpga_kernel_${kernel_name})
	endforeach()
endif(SUPPORT_FPGA)
