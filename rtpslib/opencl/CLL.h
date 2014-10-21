
#ifndef RTPS_CL_H_INCLUDED
#define RTPS_CL_H_INCLUDED

#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>

#ifdef WIN32
    #if defined(rtps_EXPORTS)
        #define RTPS_EXPORT __declspec(dllexport)
    #else
        #define RTPS_EXPORT __declspec(dllimport)
	#endif 
#else
    #define RTPS_EXPORT
#endif

namespace rtps
{
    //NVIDIA helper functions    
    RTPS_EXPORT const char* oclErrorString(cl_int error);
    RTPS_EXPORT cl_int oclGetPlatformID(cl_platform_id* clSelectedPlatformID);

    class RTPS_EXPORT CL
    {
    public:
        CL();
        cl::Context context;
        cl::CommandQueue queue;

        std::vector<cl::Device> devices;
        int deviceUsed;

        int err;
        cl::Event event;

        //setup an OpenCL context that shares with OpenGL
        void setup_gl_cl();

        cl::Program loadProgram(std::string path, std::string options="");
        cl::Kernel loadKernel(std::string path, std::string name);
        cl::Kernel loadKernel(cl::Program program, std::string kernel_name);

        void addIncludeDir(std::string);

    private:
        std::string inc_dir;
    };







}

#endif

