
#ifndef RTPS_KERNEL_H_INCLUDED
#define RTPS_KERNEL_H_INCLUDED

#include <string>
#include <stdio.h>

#include <opencl/CLL.h>
#include <rtps_common.h>

namespace rtps
{


    class RTPS_EXPORT Kernel
    {
    public:
        Kernel()
        {
            cli = NULL;
        };
        Kernel(CL *cli, std::string source, std::string name);
        Kernel(CL *cli, cl::Program program, std::string name);

        std::string name;
        std::string source;

        CL *cli;
        cl::Program program;

        cl::Kernel kernel;

        template <class T> void setArg(int arg, T val);
        void setArgShared(int arg, int nb_bytes);

        float execute(int ndrange);
        float execute(int ndrange, int workgroup_size);

    };

    template <class T> void Kernel::setArg(int arg, T val)
    {
        try
        {
            kernel.setArg(arg, val);
        }
        catch (cl::Error er)
        {
            printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
        }

    }



}

#endif

