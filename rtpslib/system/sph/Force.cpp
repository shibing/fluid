
#include <SPH.h>
#include <iostream>

namespace rtps 
{

    //----------------------------------------------------------------------
    Force::Force(std::string path, CL* cli_)
    {
        cli = cli_;
        try
        {
            path = path + "/force.cl";
            k_force = Kernel(cli, path, "force_update");
            std::cout  << "Load Force kernel" << std::endl;
        }
        catch (cl::Error er)
        {
            printf("ERROR(Force): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
    }

    void Force::execute(int num,
                    Buffer<float4>& pos_s,
                    Buffer<float>& dens_s,
                    Buffer<float4>& veleval_s,
                    Buffer<float4>& force_s,
                    Buffer<float4>& xsph_s,
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_end,
                    //params
                    Buffer<SPHParams>& sphp,
                    Buffer<GridParams>& gp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    { 
        int iarg = 0;
        k_force.setArg(iarg++, pos_s.getDevicePtr());
        k_force.setArg(iarg++, dens_s.getDevicePtr());
        k_force.setArg(iarg++, veleval_s.getDevicePtr());
        k_force.setArg(iarg++, force_s.getDevicePtr());
        k_force.setArg(iarg++, xsph_s.getDevicePtr());
        k_force.setArg(iarg++, ci_start.getDevicePtr());
        k_force.setArg(iarg++, ci_end.getDevicePtr());
        k_force.setArg(iarg++, gp.getDevicePtr());
        k_force.setArg(iarg++, sphp.getDevicePtr());

        // ONLY IF DEBUGGING
        k_force.setArg(iarg++, clf_debug.getDevicePtr());
        k_force.setArg(iarg++, cli_debug.getDevicePtr());

        int local = 64;
        try
        {
            float gputime = k_force.execute(num, local);
        }

        catch (cl::Error er)
        {
            printf("ERROR(force ): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
    }
}

