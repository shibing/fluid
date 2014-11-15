CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

FILE(GLOB cl_cpy "cl_src/*")

message("bin= ${sphlib_BINARY_DIR}")
MAKE_DIRECTORY(${sphlib_BINARY_DIR}/cl_sph)

FOREACH(loop_var ${cl_cpy})
    FILE(COPY ${loop_var} DESTINATION ${sphlib_BINARY_DIR}/cl_sph)
ENDFOREACH(loop_var)

IF(PREPROCESS_CL)
    IF(NOT WIN32)
        FILE(GLOB cl_src "${sphlib_BINARY_DIR}/cl_sph/*.cl")
        FILE(MAKE_DIRECTORY ${sphlib_BINARY_DIR}/cl_sph/tmp)
        FOREACH(loop_var ${cl_src})
            get_filename_component(filename ${loop_var} NAME)
            EXECUTE_PROCESS(COMMAND ${sphlib_SOURCE_DIR}/utils/compile_to_cl.x ${filename} ${sphlib_BINARY_DIR}/cl_sph/tmp
                WORKING_DIRECTORY ${sphlib_BINARY_DIR}/cl_sph
            )
            message("working dir :  ${CMAKE_CURRENT_SOURCE_DIR}")
        ENDFOREACH(loop_var)
        
        FILE(GLOB cl_cpy "${sphlib_BINARY_DIR}/cl_sph/tmp/*")
        FOREACH(loop_var ${cl_cpy})
            FILE(COPY ${loop_var} DESTINATION ${sphlib_BINARY_DIR}/cl_sph)
        ENDFOREACH(loop_var)

        FILE(REMOVE_RECURSE ${sphlib_BINARY_DIR}/cl_sph/tmp)
    ENDIF(NOT WIN32)
ENDIF(PREPROCESS_CL)

FILE(MAKE_DIRECTORY ${SPHFLUID_INSTALL_DIR}/cl_sph)
INSTALL(CODE "FILE(MAKE_DIRECTORY ${SPHFLUID_INSTALL_DIR}/cl_sph)") 
FILE(GLOB cl_cpy "${sphlib_BINARY_DIR}/cl_sph/*")
message("cl_cpy: ${cl_cpy}")
install(FILES ${cl_cpy} DESTINATION ${SPHFLUID_INSTALL_DIR}/cl_sph)

#ENDIF(WIN32)
