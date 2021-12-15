cmake_minimum_required (VERSION 2.6)

# specify the cross compiler
SET(CMAKE_C_COMPILER   icc)
SET(CMAKE_CXX_COMPILER icpc)
SET(MPI_C_COMPILER mpiicc)
SET(_CMAKE_TOOLCHAIN_PREFIX  x86_64-k1om-linux-)

# Añadimos los paquetes necesarios para la creación del make
find_package(MPI REQUIRED)
find_package(CUDA QUIET REQUIRED)
find_package(OpenMP)

#set(CUDA_TOOLKIT_ROOT_DIR "/usr/local/cuda-8.0")
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR k1om)
SET(CMAKE_SYSTEM_VERSION 1)


# where is the target environment 
SET(CMAKE_FIND_ROOT_PATH /usr/linux-k1om-4.7)

# Configuramos el proyecto
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -O3 -fp-model precise -fp-model except")
set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -arch=compute_52 -I/usr/local/cuda-8.0/samples/common/inc -ccbin=icc -fp-model precise -fp-model except")
#set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -arch=compute_52")

if (OPENMP_FOUND)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif(OPENMP_FOUND)

# Directorios de inclusion
include_directories(${HITMAP_PATH}/include)
include_directories(${MPI_INCLUDE_PATH})
include_directories(${PROJECT_SOURCE_DIR}/calXPHI/inc)
include_directories(${PROJECT_SOURCE_DIR}/cal/inc)

# Indicamos los ficheros

# Creamos los binarios

#CPU reference
add_executable(mandelbrotREF src/mandelbrotCPU.c)

#XPHI reference
add_executable(mandelbrotXPHI src/mandelbrotXPHI.c)

#XPHI controller
cuda_add_executable(mandelbrotCntrlXPHI src/mandelbrotCntrlXPHI.c)
cuda_add_cublas_to_target(mandelbrotCntrlXPHI)
target_link_libraries(mandelbrotCntrlXPHI ${MPI_LIBRARIES} ${HITMAPLIB} -lm cal calXPHIicc)


