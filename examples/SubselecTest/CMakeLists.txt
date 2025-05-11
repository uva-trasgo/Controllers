cmake_minimum_required(VERSION 3.20)

if(SUPPORT_FPGA)
	set(
		CMAKE_C_FLAGS
		"${CMAKE_C_FLAGS} -DKERNELTYPE=TASK -DCTRL_KERNEL_PATH=${CTRL_KERNELS_PATH_FPGA} -DREF_KERNEL_PATH=${REF_KERNELS_PATH_FPGA}"
	)
	set(
		CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} -DKERNELTYPE=TASK -DCTRL_KERNEL_PATH=${CTRL_KERNELS_PATH_FPGA} -DREF_KERNEL_PATH=${REF_KERNELS_PATH_FPGA}"
	)
endif(SUPPORT_FPGA)

if(CTRL_CMAKE_VERBOSE_EXAMPLES_SUBSELECTEST)
	message(STATUS "TEST_INCLUDE DIRS = ${CTRL_INCLUDE_DIRS}")
	message(STATUS "TEST_LIBS = ${CTRL_LIBS}")
	message(STATUS "TEST_CTRL_C vars = ${CMAKE_C_FLAGS}")
	message(STATUS "TEST_CTRL_CXX vars = ${CMAKE_CXX_FLAGS}")
	message(STATUS "TEST_CTRL_CUDA vars = ${CMAKE_CUDA_FLAGS}")
endif(CTRL_CMAKE_VERBOSE_EXAMPLES_SUBSELECTEST)

include_directories(${CTRL_INCLUDE_DIRS})

if(SUPPORT_CUDA)
	add_executable(SubselecTest_Ctrl src/SubselecTest_Ctrl.cu)
elseif(SUPPORT_HIP)
	add_executable(SubselecTest_Ctrl src/SubselecTest_Ctrl.cpp)
else(SUPPORT_CUDA)
	add_executable(SubselecTest_Ctrl src/SubselecTest_Ctrl.c)
endif(SUPPORT_CUDA)

if(SUPPORT_FPGA)
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/kernels_fpga.aocx
		COMMAND
			aoc ${AOC_CTRL_DEFS} ${AOC_CTRL_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/src/kernels_fpga.cl -o
			${CMAKE_CURRENT_SOURCE_DIR}/src/kernels_fpga.aocx ${FPGA_AOC_FLAGS}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/kernels_fpga.cl
	)
	add_custom_target(
		SubselecTest_Kernels_FPGA
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/kernels_fpga.aocx ${CMAKE_CURRENT_SOURCE_DIR}/src/kernels_fpga.cl
	)
	add_dependencies(SubselecTest_Ctrl SubselecTest_Kernels_FPGA)
endif(SUPPORT_FPGA)
target_link_libraries(SubselecTest_Ctrl ${CTRL_LIBS})

add_executable(SubselecTest_Ref src/SubselecTest_Ref.c)
target_link_libraries(SubselecTest_Ref ${CTRL_LIBS})
