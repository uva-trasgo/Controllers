cmake_minimum_required (VERSION 2.6)
# specify the cross compiler
SET(CMAKE_C_COMPILER   icc)
SET(CMAKE_CXX_COMPILER icpc)

# Añadimos los paquetes necesarios para la creación del make
find_package(CUDA QUIET REQUIRED)
find_package(MPI REQUIRED)
find_package(OpenMP)
find_package(Doxygen)

#set(CUDA_TOOLKIT_ROOT_DIR "/usr/local/cuda-8.0")
# Configuramos el proyecto
set(VERSION_MAJOR    2)
set(VERSION_MINOR    0)
set(VERSION_REVISION 0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 ")
if (OPENMP_FOUND)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif(OPENMP_FOUND)

# Indicamos los ficheros
include_directories(${HITMAP_PATH}/include)
#include_directories(/usr/local/cuda-8.0/include)
include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${MPI_INCLUDE_PATH})

include_directories(../../calKernels)
include_directories(../inc)
file(GLOB HEADERS ../inc/*.h ../../calKernels/*.h)
file(GLOB SOURCES ../src/*.c ../src/*.h)

# Creamos los binarios
add_library(calXPHIicc SHARED ${SOURCES} ${HEADERS})
set_target_properties(calXPHIicc PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION} SOVERSION ${VERSION_MAJOR})


# Configuramos la instalación
install(TARGETS calXPHIicc DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME})

# Creamos los test
