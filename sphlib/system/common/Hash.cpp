#include <iostream>

#include <Hash.h>
#include <render/Render.h>
#include <RTPSettings.h>
#include <RTPS.h>
#include <CLL.h>
#include <string>

namespace rtps
{

    Hash::Hash(const std::string& path, CL* cli_)
    {
        std::cout << path << std::endl;
        cli = cli_;
        k_hash = Kernel(cli, path + "/hash.cl", "hash");
        std::cout << "Load hash kernel" << std::endl;
    }

    void Hash::execute(int num,
                    //input
                    Buffer<float4>& pos_u, 
                    //output
                    Buffer<unsigned int>& hashes,
                    Buffer<unsigned int>& indices,
                    //params
                    Buffer<GridParams>& gp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {

        int args = 0;
        //k_hash.setArg(args++, uvars.getDevicePtr()); // positions + other variables
        k_hash.setArg(args++, num);
        k_hash.setArg(args++, pos_u.getDevicePtr()); 
        k_hash.setArg(args++, hashes.getDevicePtr());
        k_hash.setArg(args++, indices.getDevicePtr());
        k_hash.setArg(args++, gp.getDevicePtr());
        k_hash.setArg(args++, clf_debug.getDevicePtr());
        k_hash.setArg(args++, cli_debug.getDevicePtr());

		
        int ctaSize = 128; // work group size
        float gputime = k_hash.execute(num, ctaSize);
   }

    //----------------------------------------------------------------------

}
