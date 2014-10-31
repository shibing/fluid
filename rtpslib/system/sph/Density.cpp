

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
                    //input
                    //Buffer<float4>& svars, 
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
    float SPH::Wpoly6(float4 r, float h)
    {
        float h9 = h*h*h * h*h*h * h*h*h;
        float alpha = 315.f/64.0f/sphp.PI/h9;
        float r2 = dist_squared(r);
        float hr2 = (h*h - r2);
        float Wij = alpha * hr2*hr2*hr2;
        return Wij;
    }

    void SPH::cpuDensity()
    {
        float h = sphp.smoothing_distance;
        float scale = sphp.simulation_scale;
        float sum_densities = 0.0f;

        for (int i = 0; i < num; i++)
        {
            float4 p = positions[i];
            p = float4(p.x * scale, p.y * scale, p.z * scale, p.w * scale);
            densities[i] = 0.0f;

            int neighbor_count = 0;
            for (int j = 0; j < num; j++)
            {
                if (j == i) continue;
                float4 pj = positions[j];
                pj = float4(pj.x * scale, pj.y * scale, pj.z * scale, pj.w * scale);
                float4 r = float4(p.x - pj.x, p.y - pj.y, p.z - pj.z, p.w - pj.w);
                //error[i] = r;
                float rlen = magnitude(r);
                if (rlen < h)
                {
                    float r2 = dist_squared(r);
                    float re2 = h*h;
                    neighbor_count++;
                    float Wij = Wpoly6(r, h);
                    densities[i] += sphp.mass * Wij;
                }

            }
        }
    }

}
