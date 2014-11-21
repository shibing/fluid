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
                    Buffer<GridParams>& gp,
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
        k_leapfrog.setArg(iargs++, gp.getDevicePtr());
        k_leapfrog.setArg(iargs++, sphp.getDevicePtr());
        k_leapfrog.setArg(iargs++, dt); //time step

        int local_size = 128;
        float gputime = k_leapfrog.execute(num, local_size);
    } 

}

} //namespace rtps
