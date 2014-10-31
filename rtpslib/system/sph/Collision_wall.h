
#ifndef RTPS_COLLISION_WALL_H_INCLUDED
#define RTPS_COLLISION_WALL_H_INCLUDED


#include <CLL.h>
#include <Buffer.h>


namespace rtps
{
    class CollisionWall
    {
        public:
            CollisionWall() { cli = NULL;};
            CollisionWall(std::string path, CL* cli);
            void execute(int num,
                        Buffer<float4>& pos_s, 
                        Buffer<float4>& vel_s, 
                        Buffer<float4>& force_s, 
                        //Buffer<float4>& svars, 
                        //params
                        Buffer<SPHParams>& sphp,
                        Buffer<GridParams>& gp,
                        //debug
                        Buffer<float4>& clf_debug,
                        Buffer<int4>& cli_debug);
        private:
            CL* cli;
            Kernel k_collision_wall;
    };
}

#endif
