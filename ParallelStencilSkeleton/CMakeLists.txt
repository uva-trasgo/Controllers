cmake_minimum_required (VERSION 3.17)

# Common CUDA options
if (SUPPORT_CUDA)
	set(CTRL_EXAMPLES_CUDA_ERROR_CHECK ON) # Check errors with this
	set(CTRL_EXAMPLES_CUDA_DEBUG OFF) # Debug with this
	set(CUDA_PROPAGATE_HOST_FLAGS ON)
endif(SUPPORT_CUDA)

# Apply options
if (CTRL_EXAMPLES_CUDA_ERROR_CHECK)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_CUDA_ERROR_CHECK_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_CUDA_ERROR_CHECK_ ")
endif (CTRL_EXAMPLES_CUDA_ERROR_CHECK)

if (CTRL_EXAMPLES_CUDA_DEBUG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_CUDA_DEBUG_")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_CUDA_DEBUG_")
endif (CTRL_EXAMPLES_CUDA_DEBUG)
	
# Verbose
if (CTRL_CMAKE_VERBOSE_EXAMPLES)
	message(STATUS "PARALLEL_STENCIL_SKELETON_INCLUDE DIRS = ${CTRL_INCLUDE_DIRS}")
	message(STATUS "PARALLEL_STENCIL_SKELETON_LIBS = ${CTRL_LIBS}")
	message(STATUS "PARALLEL_STENCIL_SKELETON_CTRL_C vars = ${CMAKE_C_FLAGS}")
	if (SUPPORT_CUDA)
		message(STATUS "PARALLEL_STENCIL_SKELETON_CTRL_CUDA vars = ${CMAKE_CUDA_FLAGS}")
	endif (SUPPORT_CUDA)
endif(CTRL_CMAKE_VERBOSE_EXAMPLES)
	
# Include directories and libs
include_directories(${CTRL_INCLUDE_DIRS} src/)
set(CTRL_LIBS ${CTRL_LIBS} Ctrl)

# Compile kernels with nvcc if CUDA is active. If not, compile them with the C compiler
if (SUPPORT_CUDA)
	set(EPSILOD_KERNELS "src/epsilod_kernels.cu")
	set(TEST_KERNEL_SOURCE "src/test_parallelStencilSkeleton_kernels.cu")
else (SUPPORT_CUDA)
	set(EPSILOD_KERNELS "src/epsilod_kernels.c")
	set(TEST_KERNEL_SOURCE "src/test_parallelStencilSkeleton_kernels.c")
endif(SUPPORT_CUDA)

add_executable(test_parallelStencilSkeleton ${EPSILOD_KERNELS} ${TEST_KERNEL_SOURCE} src/test_parallelStencilSkeleton.c src/epsilod.c)
target_link_libraries(test_parallelStencilSkeleton ${CTRL_LIBS})

add_executable(testing_selectIOTiles ${EPSILOD_KERNELS} ${TEST_KERNEL_SOURCE} src/ALB_Testing/selectIOTiles/testing_selectIOTiles.c)
target_link_libraries(testing_selectIOTiles ${CTRL_LIBS})

add_executable(testing_computeCommonData ${EPSILOD_KERNELS} ${TEST_KERNEL_SOURCE} src/ALB_Testing/computeCommonData/testing_computeCommonData.c)
target_link_libraries(testing_computeCommonData ${CTRL_LIBS})
add_executable(testing_computeCommonData_InvShp ${EPSILOD_KERNELS} ${TEST_KERNEL_SOURCE} src/ALB_Testing/computeCommonData/testing_computeCommonDataInvalidShape.c)
target_link_libraries(testing_computeCommonData_InvShp ${CTRL_LIBS})
add_executable(testing_computeCommonData_InvBord ${EPSILOD_KERNELS} ${TEST_KERNEL_SOURCE} src/ALB_Testing/computeCommonData/testing_computeCommonDataInvalidBorders.c)
target_link_libraries(testing_computeCommonData_InvBord ${CTRL_LIBS})

add_executable(testing_computeSingleTileData ${EPSILOD_KERNELS} ${TEST_KERNEL_SOURCE} src/ALB_Testing/computeSingleTileData/testing_computeSingleTileData.c)
target_link_libraries(testing_computeSingleTileData ${CTRL_LIBS})
add_executable(testing_computeSingleTileData_InvShp ${EPSILOD_KERNELS} ${TEST_KERNEL_SOURCE} src/ALB_Testing/computeSingleTileData/testing_computeSingleTileDataInvalidShape.c)
target_link_libraries(testing_computeSingleTileData_InvShp ${CTRL_LIBS})
add_executable(testing_computeSingleTileData_InvBord ${EPSILOD_KERNELS} ${TEST_KERNEL_SOURCE} src/ALB_Testing/computeSingleTileData/testing_computeSingleTileDataInvalidBorder.c)
target_link_libraries(testing_computeSingleTileData_InvBord ${CTRL_LIBS})

add_executable(testing_expandBorders ${EPSILOD_KERNELS} ${TEST_KERNEL_SOURCE} src/ALB_Testing/expandBorders/testing_expandBorders.c)
target_link_libraries(testing_expandBorders ${CTRL_LIBS})

add_executable(testing_hit_pattern ${EPSILOD_KERNELS} ${TEST_KERNEL_SOURCE} src/ALB_Testing/hit_pattern/testing_hit_pattern.c)
target_link_libraries(testing_hit_pattern ${CTRL_LIBS})

add_executable(testing_alb ${EPSILOD_KERNELS} ${TEST_KERNEL_SOURCE} src/ALB_Testing/alb/testing_alb.c)
target_link_libraries(testing_alb ${CTRL_LIBS})


