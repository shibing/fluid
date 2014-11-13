IF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	MESSAGE(FATAL_ERROR "CMake generation must always be out-of-source!
    Remove the CMakeCache.txt file and try again from another folder")
ENDIF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(rtpslib C CXX)

SET( CMAKE_MODULE_PATH ${RTPS_CMAKE_MODULE_PATH})

SET(RTPS_DEBUG TRUE)

#only works on mac/linux
SET(PREPROCESS_CL TRUE)

SET(WITH_OPENCL TRUE)

ADD_DEFINITIONS(-g )

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


set (rtpslib_INCLUDES
    ${OPENGL_INCLUDE_DIR}
    ${OPENCL_INCLUDE_DIR}
    ${rtpslib_SOURCE_DIR}
    ${rtpslib_SOURCE_DIR}/system
    ${rtpslib_SOURCE_DIR}/system/sph
    ${rtpslib_SOURCE_DIR}/system/common
    ${rtpslib_SOURCE_DIR}/render
    ${rtpslib_SOURCE_DIR}/domain
    ${rtpslib_SOURCE_DIR}/opencl
    ${rtpslib_SOURCE_DIR}/matrix_library
)    

set (rtpslib_SOURCES
    ${rtpslib_SOURCE_DIR}/RTPS
    ${rtpslib_SOURCE_DIR}/RTPSettings
    ${rtpslib_SOURCE_DIR}/util
    ${rtpslib_SOURCE_DIR}/structs
    ${rtpslib_SOURCE_DIR}/timege    #gordon's timer class
    ${rtpslib_SOURCE_DIR}/timer_eb  #evan's timer class
    ${rtpslib_SOURCE_DIR}/rtps_common

    ${rtpslib_SOURCE_DIR}/render/Render
    ${rtpslib_SOURCE_DIR}/render/text
    ${rtpslib_SOURCE_DIR}/render/quad
    ${rtpslib_SOURCE_DIR}/render/cube


    ${rtpslib_SOURCE_DIR}/domain/Domain
    ${rtpslib_SOURCE_DIR}/domain/IV

    ${rtpslib_SOURCE_DIR}/system/common/Hose
    ${rtpslib_SOURCE_DIR}/system/common/Hash
    ${rtpslib_SOURCE_DIR}/system/common/CellIndices
    ${rtpslib_SOURCE_DIR}/system/common/Permute
	
	# SPH
    ${rtpslib_SOURCE_DIR}/system/SPH.cpp
    ${rtpslib_SOURCE_DIR}/system/SPHSettings.cpp
    ${rtpslib_SOURCE_DIR}/system/sph/Euler
    ${rtpslib_SOURCE_DIR}/system/sph/LeapFrog
    ${rtpslib_SOURCE_DIR}/system/sph/Density
    ${rtpslib_SOURCE_DIR}/system/sph/Force
    ${rtpslib_SOURCE_DIR}/system/sph/Collision_wall
    ${rtpslib_SOURCE_DIR}/system/sph/Collision_triangle
    
    )

SET(rtpslib_SOURCES ${rtpslib_SOURCES}
    ${rtpslib_SOURCE_DIR}/opencl/CLL
    ${rtpslib_SOURCE_DIR}/opencl/Kernel
    )
INCLUDE_DIRECTORIES(${rtpslib_INCLUDES})
ADD_LIBRARY(rtps STATIC ${rtpslib_SOURCES})

TARGET_LINK_LIBRARIES (rtps
   ${OPENGL_LIBRARIES}
   ${OPENCL_LIBRARIES}
   Qt5::Gui
)

ADD_SUBDIRECTORY(system/common)
ADD_SUBDIRECTORY(system/sph)

MAKE_DIRECTORY(${rtpslib_BINARY_DIR}/shaders)
MAKE_DIRECTORY(${rtpslib_BINARY_DIR}/textures)
MAKE_DIRECTORY(${rtpslib_BINARY_DIR}/models)
FILE(COPY ${rtpslib_SOURCE_DIR}/render/shaders/ DESTINATION ${rtpslib_BINARY_DIR}/shaders/)
FILE(COPY ${rtpslib_SOURCE_DIR}/render/textures/ DESTINATION ${rtpslib_BINARY_DIR}/textures/)
FILE(COPY ${rtpslib_SOURCE_DIR}/render/models/ DESTINATION ${rtpslib_BINARY_DIR}/models/)

INSTALL (TARGETS rtps DESTINATION ${sphfluid_BINARY_DIR}/bin)
INSTALL(CODE "FILE(MAKE_DIRECTORY ${SPHFLUID_INSTALL_DIR}/shaders)")
INSTALL(CODE "FILE(MAKE_DIRECTORY ${SPHFLUID_INSTALL_DIR}/textures)")
INSTALL(CODE "FILE(MAKE_DIRECTORY ${SPHFLUID_INSTALL_DIR}/models)")

INSTALL (DIRECTORY ${rtpslib_BINARY_DIR}/shaders/ DESTINATION ${SPHFLUID_INSTALL_DIR}/shaders/)
INSTALL (DIRECTORY ${rtpslib_BINARY_DIR}/textures/ DESTINATION ${SPHFLUID_INSTALL_DIR}/textures/)
INSTALL (DIRECTORY ${rtpslib_BINARY_DIR}/models/ DESTINATION ${SPHFLUID_INSTALL_DIR}/models/)
