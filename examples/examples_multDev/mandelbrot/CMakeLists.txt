cmake_minimum_required (VERSION 2.6)

# Añadimos los paquetes necesarios para la creación del make
find_package(MPI REQUIRED)
find_package(CUDA QUIET REQUIRED)
find_package(OpenMP)

# Configuramos el proyecto
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -O3")
#set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -arch=compute_52 -I/usr/local/cuda/samples/common/inc -Xcompiler -fopenmp ")
set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -I/usr/local/cuda/samples/common/inc -Xcompiler -fopenmp -O3 ")
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
cuda_add_executable(mandelbrotCntrlDev src/mandelbrotCntrlDev.cu)
cuda_add_cublas_to_target(mandelbrotCntrlDev)
target_link_libraries(mandelbrotCntrlDev ${MPI_LIBRARIES} ${HITMAPLIB} -lm cal calXPHIgcc)

cuda_add_executable(mandelbrot2GPUs src/mandelbrot2GPUs.cu)
cuda_add_cublas_to_target(mandelbrot2GPUs)
target_link_libraries(mandelbrot2GPUs ${MPI_LIBRARIES} ${HITMAPLIB} -lm cal calXPHIgcc)

cuda_add_executable(mandelbrot1GPU src/mandelbrot1GPU.cu)
cuda_add_cublas_to_target(mandelbrot1GPU)
target_link_libraries(mandelbrot1GPU ${MPI_LIBRARIES} ${HITMAPLIB} -lm cal calXPHIgcc)

cuda_add_executable(mandelbrotCudaR2 src/mandelbrotCuda2.cu)
cuda_add_cublas_to_target(mandelbrotCudaR2)
target_link_libraries(mandelbrotCudaR2 ${MPI_LIBRARIES} ${HITMAPLIB} -lm cal calXPHIgcc)

cuda_add_executable(mandelbrotCudaR src/mandelbrotCuda.cu)
cuda_add_cublas_to_target(mandelbrotCudaR)
target_link_libraries(mandelbrotCudaR ${MPI_LIBRARIES} ${HITMAPLIB} -lm cal calXPHIgcc)


