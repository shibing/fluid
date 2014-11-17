
#ifndef RTPS_DENSITY_H_INCLUDED
#define RTPS_DENSITY_H_INCLUDED


#include <CLL.h>
#include <Buffer.h>
#include <system/SPHSettings.h>


namespace rtps
{
    class Density
    {
        public:
            Density() { cli = NULL; }
            Density(std::string path, CL* cli);
            void execute(int num,
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
                    Buffer<int4>& cli_debug);

        private:
            CL* cli;
            Kernel k_density;
    };
}

#endif
