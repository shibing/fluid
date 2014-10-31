
#ifndef RTPS_HASH_H_INCLUDED
#define RTPS_HASH_H_INCLUDED


#include <system/SPHSettings.h>
#include <RTPS.h>
#include <Buffer.h>
#include <opencl/Kernel.h>

namespace rtps
{

    class CL;
    

    class Hash
    {
        public:
            Hash() { cli = NULL; }
            Hash(std::string path, CL* cli);
            void execute(int num,
                        Buffer<float4>& pos_u, 
                        //output
                        Buffer<unsigned int>& hashes,
                        Buffer<unsigned int>& indices,
                        //params
                        //Buffer<SPHParams>& sphp,
                        Buffer<GridParams>& gp,
                        //debug
                        Buffer<float4>& clf_debug,
                        Buffer<int4>& cli_debug);
        private:
            CL* cli;
            Kernel k_hash;
    };
}

#endif
