cmake_minimum_required (VERSION 2.6)
project (cal)

set(HITMAP_PATH "${PROJECT_SOURCE_DIR}/extern/hitmap")

list(APPEND HITMAP_INCLUDE_PATHS "${HITMAP_PATH}/include")
list(APPEND HITMAP_INCLUDE_PATHS "${HITMAP_PATH}/extern/iohb1.0/")
list(APPEND HITMAP_INCLUDE_PATHS "${HITMAP_PATH}/extern/matrix_io/")
list(APPEND HITMAP_INCLUDE_PATHS "${HITMAP_PATH}/extern/metis/Lib/")

find_library( HITMAPLIB hit HINTS ${HITMAP_PATH}/lib )
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMPICH_SKIP_MPICXX  -DMPI_NO_CPPBIND -DSGIMPI ")
# Configuramos el proyecto

#Añadimos los directorios
add_subdirectory(cal)
add_subdirectory(calXPHI)

####### EXAMPLES ########
add_subdirectory(examples)
# add_subdirectory(examples_gpu)
# add_subdirectory(examples_multDev)
#Xeon Phi
#add_subdirectory(calXPHI/ICC)
#add_subdirectory(examples_xphi)
