IF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	MESSAGE(FATAL_ERROR "CMake generation must always be out-of-source!
    Remove the CMakeCache.txt file and try again from another folder")
ENDIF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(sphlib C CXX)

SET(CMAKE_MODULE_PATH ${SPHFLUID_CMAKE_MODULE_PATH})

SET(SPHFLUID_DEBUG TRUE)

#only works on mac/linux
SET(PREPROCESS_CL TRUE)

SET(WITH_OPENCL TRUE)

ADD_DEFINITIONS(-g)

option(WITH_SHOW_DENSITY "" ON)
option(WITH_SHOW_FORCE "" ON)
IF(WITH_SHOW_DENSITY)
    ADD_DEFINITIONS(-DSHOW_DENSITY)
ENDIF()

IF(WITH_SHOW_FORCE)
    ADD_DEFINITIONS(-DSHOW_FORCE)
ENDIF()

##### Source Paths ############################################################
ADD_DEFINITIONS(-DSPH_CL_SOURCE_DIR="cl_sph")
ADD_DEFINITIONS(-DCOMMON_CL_SOURCE_DIR="cl_common")
ADD_DEFINITIONS(-DGLSL_BIN_DIR="shaders")
ADD_DEFINITIONS(-DGLSL_SOURCE_DIR="render")
##############################################################################


FIND_PACKAGE (OpenGL)
FIND_PACKAGE (OPENCL)
FIND_PACKAGE (Qt5Gui)
FIND_PACKAGE (Qt5Core)


set (sphlib_INCLUDES
    ${OPENGL_INCLUDE_DIR}
    ${OPENCL_INCLUDE_DIR}
    ${sphlib_SOURCE_DIR}
    ${sphlib_SOURCE_DIR}/system
    ${sphlib_SOURCE_DIR}/system/sph
    ${sphlib_SOURCE_DIR}/system/common
    ${sphlib_SOURCE_DIR}/render
    ${sphlib_SOURCE_DIR}/domain
    ${sphlib_SOURCE_DIR}/opencl
    ${sphlib_SOURCE_DIR}/matrix_library
)    

set (sphlib_SOURCES
    ${sphlib_SOURCE_DIR}/RTPS
    ${sphlib_SOURCE_DIR}/RTPSettings
    ${sphlib_SOURCE_DIR}/util
    ${sphlib_SOURCE_DIR}/structs
    ${sphlib_SOURCE_DIR}/rtps_common

    ${sphlib_SOURCE_DIR}/render/Render
    ${sphlib_SOURCE_DIR}/render/text
    ${sphlib_SOURCE_DIR}/render/quad
    ${sphlib_SOURCE_DIR}/render/cube


    ${sphlib_SOURCE_DIR}/domain/Domain
    ${sphlib_SOURCE_DIR}/domain/IV

    ${sphlib_SOURCE_DIR}/system/common/Hose
    ${sphlib_SOURCE_DIR}/system/common/Hash
    ${sphlib_SOURCE_DIR}/system/common/CellIndices
    ${sphlib_SOURCE_DIR}/system/common/Permute
	
	# SPH
    ${sphlib_SOURCE_DIR}/system/SPH.cpp
    ${sphlib_SOURCE_DIR}/system/SPHSettings.cpp
    ${sphlib_SOURCE_DIR}/system/sph/Euler
    ${sphlib_SOURCE_DIR}/system/sph/LeapFrog
    ${sphlib_SOURCE_DIR}/system/sph/Density
    ${sphlib_SOURCE_DIR}/system/sph/Force
    ${sphlib_SOURCE_DIR}/system/sph/Normal
    ${sphlib_SOURCE_DIR}/system/sph/Collision_wall
    ${sphlib_SOURCE_DIR}/system/sph/Collision_triangle
    )

SET(sphlib_SOURCES ${sphlib_SOURCES}
    ${sphlib_SOURCE_DIR}/opencl/CLL
    ${sphlib_SOURCE_DIR}/opencl/Kernel
    )
INCLUDE_DIRECTORIES(${sphlib_INCLUDES})
ADD_LIBRARY(sph STATIC ${sphlib_SOURCES})

TARGET_LINK_LIBRARIES (sph
   ${OPENGL_LIBRARIES}
   ${OPENCL_LIBRARIES}
   Qt5::Gui
)

ADD_SUBDIRECTORY(system/common)
ADD_SUBDIRECTORY(system/sph)

MAKE_DIRECTORY(${sphlib_BINARY_DIR}/shaders)
MAKE_DIRECTORY(${sphlib_BINARY_DIR}/textures)
MAKE_DIRECTORY(${sphlib_BINARY_DIR}/models)
FILE(COPY ${sphlib_SOURCE_DIR}/render/shaders/ DESTINATION ${sphlib_BINARY_DIR}/shaders/)
FILE(COPY ${sphlib_SOURCE_DIR}/render/textures/ DESTINATION ${sphlib_BINARY_DIR}/textures/)
FILE(COPY ${sphlib_SOURCE_DIR}/render/models/ DESTINATION ${sphlib_BINARY_DIR}/models/)

INSTALL (TARGETS sph DESTINATION ${sphfluid_BINARY_DIR}/bin)
INSTALL(CODE "FILE(MAKE_DIRECTORY ${SPHFLUID_INSTALL_DIR}/shaders)")
INSTALL(CODE "FILE(MAKE_DIRECTORY ${SPHFLUID_INSTALL_DIR}/textures)")
INSTALL(CODE "FILE(MAKE_DIRECTORY ${SPHFLUID_INSTALL_DIR}/models)")

INSTALL (DIRECTORY ${sphlib_BINARY_DIR}/shaders/ DESTINATION ${SPHFLUID_INSTALL_DIR}/shaders/)
INSTALL (DIRECTORY ${sphlib_BINARY_DIR}/textures/ DESTINATION ${SPHFLUID_INSTALL_DIR}/textures/)
INSTALL (DIRECTORY ${sphlib_BINARY_DIR}/models/ DESTINATION ${SPHFLUID_INSTALL_DIR}/models/)
