
#include <system/SPH.h>
namespace rtps
{

    CollisionWall::CollisionWall(std::string path, CL* cli_)
    {
        cli = cli_;
        path += "/collision_wall.cl";
        k_collision_wall = Kernel(cli, path, "collision_wall");
        std::cout << "Load Collision Wall kernel" << std::endl;
    } 
    void CollisionWall::execute(int num,
            //input
            //Buffer<float4>& svars, 
            Buffer<float4>& pos_s, 
            Buffer<float4>& vel_s, 
            Buffer<float4>& force_s, 
            //output
            //params
            Buffer<SPHParams>& sphp,
            Buffer<GridParams>& gp,
            //debug
            Buffer<float4>& clf_debug,
            Buffer<int4>& cli_debug)
    {
        int iargs = 0;
        //k_collision_wall.setArg(iargs++, svars.getDevicePtr());
        k_collision_wall.setArg(iargs++, pos_s.getDevicePtr());
        k_collision_wall.setArg(iargs++, vel_s.getDevicePtr());
        k_collision_wall.setArg(iargs++, force_s.getDevicePtr());
        k_collision_wall.setArg(iargs++, gp.getDevicePtr());
        k_collision_wall.setArg(iargs++, sphp.getDevicePtr());

        int local_size = 128;
        float gputime = k_collision_wall.execute(num, local_size);
    }
}
