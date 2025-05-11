cmake_minimum_required(VERSION 3.20)

# Apply options
if(CTRL_EXAMPLES_CUDA_ERROR_CHECK)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_CUDA_ERROR_CHECK_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_CUDA_ERROR_CHECK_ ")
endif(CTRL_EXAMPLES_CUDA_ERROR_CHECK)

if(CTRL_EXPERIMENTATION_MODE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_EXP_MODE_ ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CTRL_EXAMPLES_EXP_MODE_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_EXP_MODE_ ")
endif(CTRL_EXPERIMENTATION_MODE)

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

# Function to compile diferent versions of epsilod for each base type
function(add_epsilod_version type include)
	message(STATUS "Generating Epsilod Kernels for ${type}")
	if(SUPPORT_CUDA)
		add_library(epsilod_kernels_${type} OBJECT src/epsilod_kernels.cu)
	elseif(SUPPORT_HIP)
		add_library(epsilod_kernels_${type} OBJECT src/epsilod_kernels.cpp)
	else(SUPPORT_CUDA)
		add_library(epsilod_kernels_${type} OBJECT src/epsilod_kernels.c)
	endif(SUPPORT_CUDA)
	target_compile_definitions(epsilod_kernels_${type} PRIVATE EPSILOD_BASE_TYPE=${type} EPSILOD_TYPES_INCLUDE=\"${include}\")

	message(STATUS "Generating Epsilod for ${type}")
	add_library(epsilod_${type} OBJECT src/epsilod.c)
	target_compile_definitions(epsilod_${type} PRIVATE EPSILOD_BASE_TYPE=${type} EPSILOD_TYPES_INCLUDE=\"${include}\")
endfunction()

# Function to compile and application/example/benchmark using the appropriate base type
function(add_app name type include libs)
	message(STATUS "Generating App ${name} for type ${type}")
	if(SUPPORT_CUDA)
		add_library(${name}_kernels_${type} OBJECT src/${name}_kernels.cu)
	elseif(SUPPORT_HIP)
		add_library(${name}_kernels_${type} OBJECT src/${name}_kernels.cpp)
	else(SUPPORT_CUDA)
		add_library(${name}_kernels_${type} OBJECT src/${name}_kernels.c)
	endif(SUPPORT_CUDA)
	add_executable(
		${name}
		src/${name}.c
		$<TARGET_OBJECTS:epsilod_${type}>
		$<TARGET_OBJECTS:epsilod_kernels_${type}>
		$<TARGET_OBJECTS:${name}_kernels_${type}>
	)
	target_compile_definitions(${name} PRIVATE EPSILOD_BASE_TYPE=${type} EPSILOD_TYPES_INCLUDE=\"${include}\")
	target_compile_definitions(${name}_kernels_${type} PRIVATE EPSILOD_BASE_TYPE=${type} EPSILOD_TYPES_INCLUDE=\"${include}\")
	target_link_libraries(${name} ${CTRL_LIBS} ${libs})
endfunction()

option(EXAMPLES_USE_FLOAT "Examples use float as base type, otherwise, double" ON)
if(EXAMPLES_USE_FLOAT)
	add_compile_definitions(GASSIMULATION_USE_FLOAT)
	set(EXAMPLES_BASE_TYPE float)
else(EXAMPLES_USE_FLOAT)
	set(EXAMPLES_BASE_TYPE double)
endif(EXAMPLES_USE_FLOAT)

# Epsilod versions for different types
add_epsilod_version( float "epsilod_types.h" )
add_epsilod_version( double "epsilod_types.h" )
add_epsilod_version( cell_t "test_gassimulation_types.h" )

# Tests, examples, benchmarks, and applications
add_app( test_gassimulation cell_t "test_gassimulation_types.h" "" )
add_app( test_gaussian ${EXAMPLES_BASE_TYPE} "epsilod_types.h" "" )
add_app( test_poisson ${EXAMPLES_BASE_TYPE} "test_poisson_types.h" "" )
add_app( test_laplace ${EXAMPLES_BASE_TYPE} "test_laplace_types.h" "" )
add_app( test_wavesim ${EXAMPLES_BASE_TYPE} "test_common.h" "" )
add_app( test_parallelStencilSkeleton float "epsilod_types.h" "" )
