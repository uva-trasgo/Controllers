cmake_minimum_required (VERSION 2.6)

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
include_directories("${HITMAP_INCLUDE_PATHS}")
#include_directories(/usr/local/cuda-8.0/include)
include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${MPI_INCLUDE_PATH})
include_directories(${PROJECT_SOURCE_DIR}/calXPHI/inc)

if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile @ONLY)
endif(DOXYGEN_FOUND)

include_directories(inc)
include_directories(${PROJECT_SOURCE_DIR}/calKernels)
file(GLOB HEADERS inc/*.h ../calKernels/*.h)
file(GLOB SOURCES src/*.c src/*.h)

# Creamos los binarios
add_library(cal SHARED ${SOURCES} ${HEADERS})
#CUDA_ADD_LIBRARY(cal SHARED ${SOURCES} src/CAL_CntrlGPU.cu ${HEADERS} OPTIONS )
TARGET_LINK_LIBRARIES(cal ${CUDA_LIBRARIES})
set_target_properties(cal PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION} SOVERSION ${VERSION_MAJOR})



# Creamos la documentación
if(DOXYGEN_FOUND)
	add_custom_target(doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
		COMMENT "Generating API Documentation"
	)
endif(DOXYGEN_FOUND)

# Configuramos la instalación
install(TARGETS cal DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME})

# Creamos los test
