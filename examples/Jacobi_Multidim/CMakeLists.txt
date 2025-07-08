cmake_minimum_required(VERSION 3.20)

# Verbose
if(CTRL_CMAKE_VERBOSE_EXAMPLES_JACOBI_MULTIDIM)
	message(STATUS "JACOBI_MULTIDIM_INCLUDE DIRS = ${CTRL_INCLUDE_DIRS}")
	message(STATUS "JACOBI_MULTIDIM_LIBS = ${CTRL_LIBS}")
	message(STATUS "JACOBI_MULTIDIM_CTRL_C vars = ${CMAKE_C_FLAGS}")
	message(STATUS "JACOBI_MULTIDIM_CTRL_CXX vars = ${CMAKE_CXX_FLAGS}")
	message(STATUS "JACOBI_MULTIDIM_CTRL_CUDA vars = ${CMAKE_CUDA_FLAGS}")
endif(CTRL_CMAKE_VERBOSE_EXAMPLES_JACOBI_MULTIDIM)

# Include directories
include_directories(${CTRL_INCLUDE_DIRS})

# Compilation dependences
# Main code
set(CTRL_SOURCE_FILES src/Jacobi_Multidim_Ctrl.c)
# Generic kernel/s
if(SUPPORT_CUDA)
	set(CTRL_SOURCE_FILES ${CTRL_SOURCE_FILES} src/Jacobi_Multidim_Ctrl_Kernels.cu)
elseif(SUPPORT_HIP)
	set(CTRL_SOURCE_FILES ${CTRL_SOURCE_FILES} src/Jacobi_Multidim_Ctrl_Kernels.cpp)
else(SUPPORT_CUDA)
	set(CTRL_SOURCE_FILES ${CTRL_SOURCE_FILES} src/Jacobi_Multidim_Ctrl_Kernels.c)
endif(SUPPORT_CUDA)

add_executable(Jacobi_Multidim_Ctrl ${CTRL_SOURCE_FILES})
target_link_libraries(Jacobi_Multidim_Ctrl ${CTRL_LIBS})
