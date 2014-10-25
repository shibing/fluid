
#include <stdio.h>
#include <iostream>
#include <GL/glx.h>

#include "CLL.h"
#include "../util.h"

namespace rtps
{

    CL::CL()
    {
        inc_dir = "";
        setup_gl_cl();
    }

    void CL::addIncludeDir(std::string path)
    {
        this->inc_dir += " -I" + path;
    }

    //----------------------------------------------------------------------
    cl::Program CL::loadProgram(std::string path, std::string options)
    {

        int length;
        char* src = file_contents(path.c_str(), &length);
        std::string kernel_source(src);
        free(src);

        cl::Program program;
        try
        {
            cl::Program::Sources source(1, std::make_pair(kernel_source.c_str(), length));
            program = cl::Program(context, source);
        }
        catch (cl::Error er)
        {
            printf("loadProgram\n");
            printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
        }

        try
        {
            options += this->inc_dir;
            err = program.build(devices, options.c_str());
        }
        catch (cl::Error er)
        {
            printf("loadProgram::program.build\n");
            printf("source= %s\n", kernel_source.c_str());
            printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
        /*std::cout << "Build Status: " << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(devices.front()) << std::endl;
        std::cout << "Build Options:\t" << program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(devices.front()) << std::endl;
        std::cout << "Build Log:\t " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices.front()) << std::endl;*/

        return program;
    }

    //----------------------------------------------------------------------
    cl::Kernel CL::loadKernel(std::string path, std::string kernel_name)
    {
        cl::Program program;
        cl::Kernel kernel;
        try
        {
            program = loadProgram(path);
            kernel = cl::Kernel(program, kernel_name.c_str(), &err);
        }
        catch (cl::Error er)
        {
            printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
        return kernel;
    }

    //----------------------------------------------------------------------
    cl::Kernel CL::loadKernel(cl::Program program, std::string kernel_name)
    {
        cl::Kernel kernel;
        try
        {
            kernel = cl::Kernel(program, kernel_name.c_str(), &err);
        }
        catch (cl::Error er)
        {
            printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
        return kernel;
    }


    void CL::setup_gl_cl()
    {
        std::vector<cl::Platform> platforms;
        err = cl::Platform::get(&platforms);

        deviceUsed = 0;
        err = platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
        int t = devices.front().getInfo<CL_DEVICE_TYPE>();

       cl_context_properties props[] = 
        {
            CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(), 
            CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(), 
            CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(),
            0
        };
        try
        {
            context = cl::Context(CL_DEVICE_TYPE_GPU, props);
        }
        catch (cl::Error er)
        {
            printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
        }

        cl_command_queue_properties cq_props = CL_QUEUE_PROFILING_ENABLE;
        try
        {
            queue = cl::CommandQueue(context, devices[deviceUsed], cq_props, &err);
        }
        catch (cl::Error er)
        {
            printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
    }
    //helper function from NVIDIA
    const char* oclErrorString(cl_int error)
    {
        static const char* errorString[] = {
            "CL_SUCCESS",
            "CL_DEVICE_NOT_FOUND",
            "CL_DEVICE_NOT_AVAILABLE",
            "CL_COMPILER_NOT_AVAILABLE",
            "CL_MEM_OBJECT_ALLOCATION_FAILURE",
            "CL_OUT_OF_RESOURCES",
            "CL_OUT_OF_HOST_MEMORY",
            "CL_PROFILING_INFO_NOT_AVAILABLE",
            "CL_MEM_COPY_OVERLAP",
            "CL_IMAGE_FORMAT_MISMATCH",
            "CL_IMAGE_FORMAT_NOT_SUPPORTED",
            "CL_BUILD_PROGRAM_FAILURE",
            "CL_MAP_FAILURE",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "CL_INVALID_VALUE",
            "CL_INVALID_DEVICE_TYPE",
            "CL_INVALID_PLATFORM",
            "CL_INVALID_DEVICE",
            "CL_INVALID_CONTEXT",
            "CL_INVALID_QUEUE_PROPERTIES",
            "CL_INVALID_COMMAND_QUEUE",
            "CL_INVALID_HOST_PTR",
            "CL_INVALID_MEM_OBJECT",
            "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
            "CL_INVALID_IMAGE_SIZE",
            "CL_INVALID_SAMPLER",
            "CL_INVALID_BINARY",
            "CL_INVALID_BUILD_OPTIONS",
            "CL_INVALID_PROGRAM",
            "CL_INVALID_PROGRAM_EXECUTABLE",
            "CL_INVALID_KERNEL_NAME",
            "CL_INVALID_KERNEL_DEFINITION",
            "CL_INVALID_KERNEL",
            "CL_INVALID_ARG_INDEX",
            "CL_INVALID_ARG_VALUE",
            "CL_INVALID_ARG_SIZE",
            "CL_INVALID_KERNEL_ARGS",
            "CL_INVALID_WORK_DIMENSION",
            "CL_INVALID_WORK_GROUP_SIZE",
            "CL_INVALID_WORK_ITEM_SIZE",
            "CL_INVALID_GLOBAL_OFFSET",
            "CL_INVALID_EVENT_WAIT_LIST",
            "CL_INVALID_EVENT",
            "CL_INVALID_OPERATION",
            "CL_INVALID_GL_OBJECT",
            "CL_INVALID_BUFFER_SIZE",
            "CL_INVALID_MIP_LEVEL",
            "CL_INVALID_GLOBAL_WORK_SIZE",
        };

        const int errorCount = sizeof(errorString) / sizeof(errorString[0]);

        const int index = -error;

        return(index >= 0 && index < errorCount) ? errorString[index] : "";

    }



    //NVIDIA's code
    //////////////////////////////////////////////////////////////////////////////
    //! Gets the platform ID for NVIDIA if available, otherwise default to platform 0
    //!
    //! @return the id 
    //! @param clSelectedPlatformID         OpenCL platform ID
    //////////////////////////////////////////////////////////////////////////////
    cl_int oclGetPlatformID(cl_platform_id* clSelectedPlatformID)
    {
        char chBuffer[1024];
        cl_uint num_platforms;
        cl_platform_id* clPlatformIDs;
        cl_int ciErrNum;
        *clSelectedPlatformID = NULL;
        cl_uint i = 0;

        // Get OpenCL platform count
        ciErrNum = clGetPlatformIDs (0, NULL, &num_platforms);
        if (ciErrNum != CL_SUCCESS)
        {
            //shrLog(" Error %i in clGetPlatformIDs Call !!!\n\n", ciErrNum);
            printf(" Error %i in clGetPlatformIDs Call !!!\n\n", ciErrNum);
            return -1000;
        }
        else
        {
            if (num_platforms == 0)
            {
                //shrLog("No OpenCL platform found!\n\n");
                printf("No OpenCL platform found!\n\n");
                return -2000;
            }
            else
            {
                // if there's a platform or more, make space for ID's
                if ((clPlatformIDs = (cl_platform_id*)malloc(num_platforms * sizeof(cl_platform_id))) == NULL)
                {
                    //shrLog("Failed to allocate memory for cl_platform ID's!\n\n");
                    printf("Failed to allocate memory for cl_platform ID's!\n\n");
                    return -3000;
                }

                // get platform info for each platform and trap the NVIDIA platform if found
                ciErrNum = clGetPlatformIDs (num_platforms, clPlatformIDs, NULL);
                for (i = 0; i < num_platforms; ++i)
                {
                    ciErrNum = clGetPlatformInfo (clPlatformIDs[i], CL_PLATFORM_NAME, 1024, &chBuffer, NULL);
                    if (ciErrNum == CL_SUCCESS)
                    {
                        if (strstr(chBuffer, "NVIDIA") != NULL)
                        {
                            *clSelectedPlatformID = clPlatformIDs[i];
                            break;
                        }
                    }
                }

                // default to zeroeth platform if NVIDIA not found
                if (*clSelectedPlatformID == NULL)
                {
                    //shrLog("WARNING: NVIDIA OpenCL platform not found - defaulting to first platform!\n\n");
                    printf("WARNING: NVIDIA OpenCL platform not found - defaulting to first platform!\n\n");
                    *clSelectedPlatformID = clPlatformIDs[0];
                }

                free(clPlatformIDs);
            }
        }

        return CL_SUCCESS;
    }




}
