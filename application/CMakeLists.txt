IF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	MESSAGE(FATAL_ERROR "CMake generation must always be out-of-source!
    Remove the CMakeCache.txt file and try again from another folder")
ENDIF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

project(sph_fluid)

SET( BASE_DIR ${sph_fluid_SOURCE_DIR}/..)
SET( LIB_DIR ${sph_fluid_SOURCE_DIR}/../sphlib)

SET( CMAKE_MODULE_PATH
     ${BASE_DIR}/cmake
     )
message("${CMAKE_MODULE_PATH}\n")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

FIND_PACKAGE (OpenGL)
FIND_PACKAGE (OPENCL)
FIND_PACKAGE (Qt5Widgets)
FIND_PACKAGE (Qt5Gui)
FIND_PACKAGE (Qt5Core)

set (APP_INCLUDES
    ${LIB_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${BASE_DIR}/opencl10
)

set (APP_SOURCES
    opengl_window
    abstract_scene
    basic_scene
    main_qt.cpp
    )

ADD_DEFINITIONS(-g )

INCLUDE_DIRECTORIES(${APP_INCLUDES})
ADD_EXECUTABLE(sph_app ${APP_SOURCES})

LINK_DIRECTORIES(${sphfluid_BINARY_DIR}/sph)

SET(LINK_LIBS
    sph
    ${OPENGL_LIBRARIES}
    ${OPENCL_LIBRARIES}
    Qt5::Gui
    Qt5::Core
)

message(RTPS_EXPORTS "${rtps_EXPORTS}")

TARGET_LINK_LIBRARIES (sph_app
    ${LINK_LIBS}
)
install (TARGETS sph_app DESTINATION ${sphfluid_BINARY_DIR}/bin)
