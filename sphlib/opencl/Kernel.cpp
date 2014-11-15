
#include <opencl/Kernel.h>

namespace rtps
{

    Kernel::Kernel(CL *cli, std::string source, std::string name)
    {
        this->cli = cli;
        this->name = name;
        this->source = source;
        kernel = cli->loadKernel(source, name);
    }

    Kernel::Kernel(CL *cli, cl::Program prog, std::string name)
    {
        this->cli = cli;
        this->name = name;
        this->program = prog;
        kernel = cli->loadKernel(program, name);
    }

    float Kernel::execute(int ndrange)
    {
        if (ndrange <= 0)
            return -1.f;

        
        cl_ulong start, end;
        float timing = -1.0f;

        try
        {
            cl::Event event;
            cli->err = cli->queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(ndrange), cl::NullRange, NULL, &event);
            cli->queue.finish();
            event.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
            event.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
            timing = (end - start) * 1.0e-6f;

        }
        catch (cl::Error er)
        {
            printf("err: work group size: %d\n", ndrange);
            printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
        return timing;

    }

    float Kernel::execute(int ndrange, int worksize)
    {
        int global;
        float factor = (1.0f * ndrange) / worksize;
        if ((int)factor != factor)
        {
            factor = (int)factor;
            global = worksize*factor + worksize;
        }
        else
        {
            global = ndrange;
        }

        if (ndrange <=0 || worksize <= 0)
            return - 1.f;

        cl_ulong start, end;
        float timing = -1.0f;

        try
        {
            cl::Event event;
            cli->err = cli->queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(global), cl::NDRange(worksize), NULL, &event);
            cli->queue.finish();
            event.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
            event.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
            timing = (end - start) * 1.0e-6f;
        }
        catch (cl::Error er)
        {
            printf("err: global %d, local %d\n", global, worksize);
            printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
        return timing;

    }

    void Kernel::setArgShared(int arg, int nb_bytes)
    {
        try
        {
            kernel.setArg(arg, nb_bytes, 0);
            cli->queue.finish();
        }
        catch (cl::Error er)
        {
            printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
    }

}
