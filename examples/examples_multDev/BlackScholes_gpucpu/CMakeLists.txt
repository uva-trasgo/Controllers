cmake_minimum_required (VERSION 2.6)

# Añadimos los paquetes necesarios para la creación del make
find_package(MPI REQUIRED)
find_package(CUDA QUIET REQUIRED)
find_package(OpenMP)

# Configuramos el proyecto
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -O3")
#set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -arch=compute_52 -I/usr/local/cuda/samples/common/inc -Xcompiler -fopenmp ")
set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -I/usr/local/cuda/samples/common/inc -Xcompiler -fopenmp -O3")
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

# Creamos los binarios

# Devices controller
cuda_add_executable(BlackScholesCntrlDev src/BlackScholesCntrlDev.cu)
cuda_add_cublas_to_target(BlackScholesCntrlDev)
target_link_libraries(BlackScholesCntrlDev ${MPI_LIBRARIES} ${HITMAPLIB} -lm cal calXPHIgcc)

cuda_add_executable(BlackScholes2GPUs src/BlackScholes2GPUs.cu)
cuda_add_cublas_to_target(BlackScholes2GPUs)
target_link_libraries(BlackScholes2GPUs ${MPI_LIBRARIES} ${HITMAPLIB} -lm cal calXPHIgcc)

cuda_add_executable(BlackScholesCudaR src/BlackScholesCuda.cu)
cuda_add_cublas_to_target(BlackScholesCudaR)
target_link_libraries(BlackScholesCudaR ${MPI_LIBRARIES} ${HITMAPLIB} -lm cal calXPHIgcc)

cuda_add_executable(BlackScholesCudaR2 src/BlackScholesCuda2.cu)
cuda_add_cublas_to_target(BlackScholesCudaR2)
target_link_libraries(BlackScholesCudaR2 ${MPI_LIBRARIES} ${HITMAPLIB} -lm cal calXPHIgcc)


