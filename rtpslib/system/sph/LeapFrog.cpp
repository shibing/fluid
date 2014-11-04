#include <iostream>
#include <system/SPH.h>
#include <RTPSettings.h>

namespace rtps
{
namespace sph
{

    LeapFrog::LeapFrog(std::string path, CL* cli_ )
    {
        cli = cli_;
        path += "/leapfrog.cl";
        k_leapfrog = Kernel(cli, path, "leapfrog");
        std::cout << "Load leap forg kernel" << std::endl;

    } 
    void LeapFrog::execute(int num,
                    float dt,
                    Buffer<float4>& pos_u,
                    Buffer<float4>& pos_s,
                    Buffer<float4>& vel_u,
                    Buffer<float4>& vel_s,
                    Buffer<float4>& veleval_u,
                    Buffer<float4>& force_s,
                    Buffer<float4>& xsph_s,
                    //params
                    Buffer<SPHParams>& sphp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {

        int iargs = 0;
        k_leapfrog.setArg(iargs++, pos_u.getDevicePtr());
        k_leapfrog.setArg(iargs++, pos_s.getDevicePtr());
        k_leapfrog.setArg(iargs++, vel_u.getDevicePtr());
        k_leapfrog.setArg(iargs++, vel_s.getDevicePtr());
        k_leapfrog.setArg(iargs++, veleval_u.getDevicePtr());
        k_leapfrog.setArg(iargs++, force_s.getDevicePtr());
        k_leapfrog.setArg(iargs++, xsph_s.getDevicePtr());
        k_leapfrog.setArg(iargs++, sphp.getDevicePtr());
        k_leapfrog.setArg(iargs++, dt); //time step

        int local_size = 128;
        float gputime = k_leapfrog.execute(num, local_size);
    } 

}

    void SPH::cpuLeapFrog()
    {
        float h = ps->settings->dt;
        for (int i = 0; i < num; i++)
        {
            float4 p = positions[i];
            float4 v = velocities[i];
            float4 f = forces[i];

            f.z += -9.8f;

            float speed = magnitude(f);
            if (speed > 600.0f) //velocity limit, need to pass in as struct
            {
                f.x *= 600.0f/speed;
                f.y *= 600.0f/speed;
                f.z *= 600.0f/speed;
            }

            float4 vnext = v;
            vnext.x += h*f.x;
            vnext.y += h*f.y;
            vnext.z += h*f.z;

            float xsphfactor = .1f;
            vnext.x += xsphfactor * xsphs[i].x;
            vnext.y += xsphfactor * xsphs[i].y;
            vnext.z += xsphfactor * xsphs[i].z;

            float scale = sphp.simulation_scale;
            p.x += h*vnext.x / scale;
            p.y += h*vnext.y / scale;
            p.z += h*vnext.z / scale;
            p.w = 1.0f; //just in case

            veleval[i].x = (v.x + vnext.x) *.5f;
            veleval[i].y = (v.y + vnext.y) *.5f;
            veleval[i].z = (v.z + vnext.z) *.5f;

            velocities[i] = vnext;
            positions[i] = p;

        }
    }

} //namespace rtps
