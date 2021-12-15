cmake_minimum_required (VERSION 2.6)

# Añadimos los paquetes necesarios para la creación del make
find_package(MPI REQUIRED)
find_package(CUDA REQUIRED)
find_package(OpenMP)

# Configuramos el proyecto
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -O3")
#set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -arch=compute_52 -I/usr/local/cuda/samples/common/inc")
set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -I/usr/local/cuda/samples/common/inc")
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
include_directories(${PROJECT_SOURCE_DIR}/calKernels)

# Creamos los binarios
#GPU Reference
cuda_add_executable(BlackScholesCuda src/BlackScholesCuda.cu)

# GPU controllers
cuda_add_executable(BlackScholesCntrlGPU src/BlackScholesCntrlGPU.cu)
cuda_add_cublas_to_target(BlackScholesCntrlGPU)
target_link_libraries(BlackScholesCntrlGPU ${MPI_LIBRARIES} ${HITMAPLIB} -lm cal calXPHIgcc)

