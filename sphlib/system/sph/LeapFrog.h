
#ifndef RTPS_LEAPFROG_H_INCLUDED
#define RTPS_LEAPFROG_H_INCLUDED


#include <CLL.h>
#include <Buffer.h>


namespace rtps
{
namespace sph
{

    class LeapFrog
    {
        public:
            LeapFrog() { cli = NULL; }
            LeapFrog(std::string path, CL* cli);
            void execute(int num,
                        float dt,
                        //input
                        Buffer<float4>& pos_u,
                        Buffer<float4>& pos_s,
                        Buffer<float4>& vel_u,
                        Buffer<float4>& vel_s,
                        Buffer<float4>& veleval_u,
                        Buffer<float4>& force_s,
                        Buffer<float4>& xsph_s,
                        Buffer<float4>& color_u,
                        Buffer<float4>& color_s,
                        //params
                        Buffer<GridParams>& gp,
                        Buffer<SPHParams>& sphp,
                        //debug
                        Buffer<float4>& clf_debug,
                        Buffer<int4>& cli_debug);

        private:
            CL* cli;
            Kernel k_leapfrog;
    };

}
}

#endif
