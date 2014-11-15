

#include <SPH.h>
#include <math.h>
#include <iostream>

namespace rtps
{
    Density::Density(std::string path, CL* cli_ )
    {
        cli = cli_;
     
        try
        {
            path = path + "/density.cl";
            k_density = Kernel(cli, path, "density_update");
            std::cout << "Load Density kernel" << std::endl;

        }
        catch (cl::Error er)
        {
            printf("ERROR(Density): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }


    }
    void Density::execute(int num,
                    Buffer<float4>& pos_s,
                    Buffer<float>& dens_s,
                    //output
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
        k_density.setArg(iarg++, pos_s.getDevicePtr());
        k_density.setArg(iarg++, dens_s.getDevicePtr());
        k_density.setArg(iarg++, ci_start.getDevicePtr());
        k_density.setArg(iarg++, ci_end.getDevicePtr());
        k_density.setArg(iarg++, gp.getDevicePtr());
        k_density.setArg(iarg++, sphp.getDevicePtr());

        k_density.setArg(iarg++, clf_debug.getDevicePtr());
        k_density.setArg(iarg++, cli_debug.getDevicePtr());

        int local = 64;
        try
        {
            float gputime = k_density.execute(num, local);
        }

        catch (cl::Error er)
        {
            printf("ERROR(density): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
    }
}
