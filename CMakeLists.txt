IF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	MESSAGE(FATAL_ERROR "CMake generation must always be out-of-source!
    Remove the CMakeCache.txt file and try again from another folder")
ENDIF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(sphfluid)

SET(SPHFLUID_CMAKE_MODULE_PATH
     ${sphfluid_SOURCE_DIR}/cmake
)
SET(OPENCL_INCLUDE_DIR 
    ${sphfluid_SOURCE_DIR}/opencl10)


set(SPHFLUID_INSTALL_DIR ${sphfluid_BINARY_DIR}/bin)
set(SPHFLUID_INSTALL_DIR_REL .)

INSTALL(CODE "FILE(MAKE_DIRECTORY ${SPHFLUID_INSTALL_DIR})")

ADD_SUBDIRECTORY (${sphfluid_SOURCE_DIR}/sphlib)
ADD_SUBDIRECTORY (${sphfluid_SOURCE_DIR}/application)
