#include <system/common/Permute.h>
#include <iostream>

#include <string>

namespace rtps
{

	//----------------------------------------------------------------------
    Permute::Permute(std::string path, CL* cli_)
    {
        cli = cli_;
        path = path + "/permute.cl";
        k_permute = Kernel(cli, path, "permute");
        std::cout << "Load Permute kernel" << std::endl;

    }

    void Permute::execute(int num,
                    //input
                    Buffer<float4>& pos_u,
                    Buffer<float4>& pos_s,
                    Buffer<float4>& vel_u,
                    Buffer<float4>& vel_s,
                    Buffer<float4>& veleval_u,
                    Buffer<float4>& veleval_s,
                    Buffer<float4>& color_u,
                    Buffer<float4>& color_s,
                    Buffer<unsigned int>& indices,
                    //params
                    //Buffer<SPHParams>& sphp,
                    Buffer<GridParams>& gp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {
        int iarg = 0;
        k_permute.setArg(iarg++, num);
        k_permute.setArg(iarg++, pos_u.getDevicePtr());
        k_permute.setArg(iarg++, pos_s.getDevicePtr());
        k_permute.setArg(iarg++, vel_u.getDevicePtr());
        k_permute.setArg(iarg++, vel_s.getDevicePtr());
        k_permute.setArg(iarg++, veleval_u.getDevicePtr());
        k_permute.setArg(iarg++, veleval_s.getDevicePtr());
        k_permute.setArg(iarg++, color_u.getDevicePtr());
        k_permute.setArg(iarg++, color_s.getDevicePtr());
        k_permute.setArg(iarg++, indices.getDevicePtr());

        int workSize = 64;
        
        try
        {
			//printf("k_permute (non-cloud): num= %d\n", num); 
            float gputime = k_permute.execute(num, workSize);

        }
        catch (cl::Error er)
        {
            printf("ERROR(data structures): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
    }
}
