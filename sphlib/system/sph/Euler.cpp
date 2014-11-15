

#include <system/SPH.h>
#include <RTPSettings.h>

namespace rtps
{
    Euler::Euler(std::string path, CL* cli_)
    {
        cli = cli_;
 
        path += "/euler.cl";
        k_euler = Kernel(cli, path, "euler");
        std::cout << "Load Euler kernel" << std::endl;
    } 
    
    void Euler::execute(int num,
                    float dt,
                    Buffer<float4>& pos_u,
                    Buffer<float4>& pos_s,
                    Buffer<float4>& vel_u,
                    Buffer<float4>& vel_s,
                    Buffer<float4>& force_s,
                    //Buffer<float4>& uvars, 
                    //Buffer<float4>& svars, 
                    Buffer<unsigned int>& indices,
                    //params
                    Buffer<SPHParams>& sphp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {

        int iargs = 0;
        //k_euler.setArg(iargs++, uvars.getDevicePtr());
        //k_euler.setArg(iargs++, svars.getDevicePtr());
        k_euler.setArg(iargs++, pos_u.getDevicePtr());
        k_euler.setArg(iargs++, pos_s.getDevicePtr());
        k_euler.setArg(iargs++, vel_u.getDevicePtr());
        k_euler.setArg(iargs++, vel_s.getDevicePtr());
        k_euler.setArg(iargs++, force_s.getDevicePtr());
        k_euler.setArg(iargs++, indices.getDevicePtr());
        k_euler.setArg(iargs++, sphp.getDevicePtr());
        k_euler.setArg(iargs++, dt); //time step

        int local_size = 128;
        k_euler.execute(num, local_size);

    }
}
