CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

FILE(GLOB cl_cpy "cl_src/*")

# copy the opencl source files to the build directory
MAKE_DIRECTORY(${sphlib_BINARY_DIR}/cl_common)

FOREACH(loop_var ${cl_cpy})
    FILE(COPY ${loop_var} DESTINATION ${sphlib_BINARY_DIR}/cl_common)
ENDFOREACH(loop_var)

IF(PREPROCESS_CL)
    IF(NOT WIN32)
        FILE(GLOB cl_src "${sphlib_BINARY_DIR}/cl_common/*.cl")
        FILE(MAKE_DIRECTORY ${sphlib_BINARY_DIR}/cl_common/tmp)
        #make_directory(${sphlib_BINARY_DIR}/cl_common)
        FOREACH(loop_var ${cl_src})
            get_filename_component(filename ${loop_var} NAME)
            EXECUTE_PROCESS(COMMAND ${sphlib_SOURCE_DIR}/utils/compile_to_cl.x ${filename} ${sphlib_BINARY_DIR}/cl_common/tmp
                WORKING_DIRECTORY ${sphlib_BINARY_DIR}/cl_common
            )
        ENDFOREACH(loop_var)
        
        FILE(GLOB cl_cpy "${sphlib_BINARY_DIR}/cl_common/tmp/*")
        FOREACH(loop_var ${cl_cpy})
            FILE(COPY ${loop_var} DESTINATION ${sphlib_BINARY_DIR}/cl_common)
        ENDFOREACH(loop_var)

        FILE(REMOVE_RECURSE ${sphlib_BINARY_DIR}/cl_common/tmp)


    ENDIF(NOT WIN32)
ENDIF(PREPROCESS_CL)

INSTALL(CODE "FILE(MAKE_DIRECTORY ${SPHFLUID_INSTALL_DIR}/cl_common)")
FILE(GLOB cl_cpy "${sphlib_BINARY_DIR}/cl_common/*")
install (FILES ${cl_cpy} DESTINATION ${SPHFLUID_INSTALL_DIR}/cl_common)

