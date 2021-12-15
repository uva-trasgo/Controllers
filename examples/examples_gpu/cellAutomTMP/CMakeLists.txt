cmake_minimum_required (VERSION 2.6)

# Añadimos los paquetes necesarios para la creación del make
find_package(MPI REQUIRED)
find_package(CUDA REQUIRED)
find_package(OpenMP)

# Configuramos el proyecto
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -arch=compute_35 -I/usr/local/cuda/samples/common/inc")
#set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -arch=compute_52")

if (OPENMP_FOUND)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif(OPENMP_FOUND)

# Directorios de inclusion
include_directories("${HITMAP_INCLUDE_PATHS}")
include_directories(${MPI_INCLUDE_PATH})
include_directories(${PROJECT_SOURCE_DIR}/calXPHI/inc)
include_directories(${PROJECT_SOURCE_DIR}/cal/inc)
include_directories(${PROJECT_SOURCE_DIR}/calKernels)

# Indicamos los ficheros

# Creamos los binarios
#GPU reference
cuda_add_executable(cellAutomCudaTMP src/cellAutomCu.cu)

#GPU controllers
cuda_add_executable(cellAutomCntrlGPUTMP src/cellAutomCntrl.cu)
cuda_add_cublas_to_target(cellAutomCntrlGPUTMP)
target_link_libraries(cellAutomCntrlGPUTMP cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")

#GPU reference
cuda_add_executable(matrixPowCudaTMP src/matrixPowCu.cu)
cuda_add_executable(matrixMultCudaTMP src/matrixMultCu.cu)

#GPU controllers
cuda_add_executable(matrixPowCntrlGPUTMP src/matrixPowCntrlGPU.cu)
cuda_add_cublas_to_target(matrixPowCntrlGPUTMP)
target_link_libraries(matrixPowCntrlGPUTMP cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)

#cuda_add_executable(jacobi2DCntrlGPU src/jacobi2DCntrlGPU.cu)
#cuda_add_cublas_to_target(jacobi2DCntrlGPU)
#target_link_libraries(jacobi2DCntrlGPU cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)


#cuda_add_executable(jacobi2DCntrlGPU3 src/jacobi2DCntrlGPU3.cu)
#cuda_add_cublas_to_target(jacobi2DCntrlGPU3)
#target_link_libraries(jacobi2DCntrlGPU3 cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)


#(versión síncrona y asíncrona. Para ello hay que tocar el define)
#cuda_add_executable(jacobi2DCntrlGPU2 src/jacobi2DCntrlGPU2.cu)
#cuda_add_cublas_to_target(jacobi2DCntrlGPU2)
#target_link_libraries(jacobi2DCntrlGPU2 cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)

################################################# (hasta 22-11-2017) Arturo cambia de ideas ....

#(versión síncrona y asíncrona. Para ello hay que tocar el define) MM
#cuda_add_executable(MMGPU src/MMGPU.cu)
#cuda_add_cublas_to_target(MMGPU)
#target_link_libraries(MMGPU cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)

#--- MM Base
#cuda_add_executable(matrixMultCuda src/matrixMultCuda.cu)
#cuda_add_cublas_to_target(matrixMultCuda)
#target_link_libraries(matrixMultCuda cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)


#(versión síncrona y asíncrona. Para ello hay que tocar el define)
#cuda_add_executable(jacobi2DCntrlGPU4 src/jacobi2DCntrlGPU4.cu)
#cuda_add_cublas_to_target(jacobi2DCntrlGPU4)
#target_link_libraries(jacobi2DCntrlGPU4 cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)



################################################# (Desde 23-11-2017) Casos mas sencillo para el EuroPar (no pienso hacer más ejemplos)

#Recurrence equation (suma de matrices sencilla: C1 = C1 +((A1+B1)*i) y C2 = C2 +((A2+B2)*i) / i= 0..n
#CUDA SYN-ASYN
#cuda_add_executable(recEqCu src/recEqCu.cu)
#cuda_add_cublas_to_target(recEqCu)
#target_link_libraries(recEqCu cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)

#Controllers
#cuda_add_executable(recEqCont src/recEqCont.cu)
#cuda_add_cublas_to_target(recEqCont)
#target_link_libraries(recEqCont cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)


#Stencil cu (sencillo, solo varia el tamaño y las iteraciones, en cada iter se manda al host para mostrar)
#cuda_add_executable(cellAutomCu src/cellAutomCu.cu)
#cuda_add_cublas_to_target(cellAutomCu)
#target_link_libraries(cellAutomCu cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)

#Stencil Controllers
#cuda_add_executable(cellAutomCntrl src/cellAutomCntrl.cu)
#cuda_add_cublas_to_target(cellAutomCntrl)
#target_link_libraries(cellAutomCntrl cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)


#MM cu (En cada iteración se multiplica y se pasa la siguiente en otro puntero diferente)
#cuda_add_executable(matrixMultCu src/matrixMultCu.cu)
#cuda_add_cublas_to_target(matrixMultCu)
#target_link_libraries(matrixMultCu cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)

#MM controller
#cuda_add_executable(matrixMultCntrl src/matrixMultCntrl.cu)
#cuda_add_cublas_to_target(matrixMultCntrl)
#target_link_libraries(matrixMultCntrl cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)

#Sobel CUDA
#cuda_add_executable(SobelCu src/SobelCu.cu)
#cuda_add_cublas_to_target(SobelCu)
#target_link_libraries(SobelCu cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)

#Sobel Ctrl
#cuda_add_executable(SobelCtrl src/SobelCtrl.cu)
#cuda_add_cublas_to_target(SobelCtrl)
#target_link_libraries(SobelCtrl cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)

################################################# (Desde 20-01-2018) SuperComputing
#BlacScholes superComputing Ctrl
#cuda_add_executable(BlackSupCtrl src/BlackSupCtrl.cu)
#cuda_add_cublas_to_target(BlackSupCtrl)
#target_link_libraries(BlackSupCtrl cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)

#Recurrence Equation superComputing Ctrl
#cuda_add_executable(recEqSupCtrl src/recEqSupCtrl.cu)
#cuda_add_cublas_to_target(recEqSupCtrl)
#target_link_libraries(recEqSupCtrl cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)

#Cannon MM superComputing Ctrl
cuda_add_executable(cannonSupCtrl src/cannonSupCtrl.cu)
cuda_add_cublas_to_target(cannonSupCtrl)
target_link_libraries(cannonSupCtrl cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)

#Jacibi 2D superComputing Ctrl
#cuda_add_executable(jacobi2DSupCtrl src/jacobi2DSupCtrl.cu)
#cuda_add_cublas_to_target(jacobi2DSupCtrl)
#target_link_libraries(jacobi2DSupCtrl cal ${MPI_LIBRARIES} ${HITMAPLIB} -lm calXPHIgcc)
