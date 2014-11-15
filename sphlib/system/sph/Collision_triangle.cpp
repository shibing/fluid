#include <iostream>
#include <system/SPH.h>

namespace rtps {

    CollisionTriangle::CollisionTriangle(std::string path, CL* cli_, int max_triangles)
    {
        cli = cli_;
        std::vector<Triangle> maxtri(max_triangles);
        cl_triangles = Buffer<Triangle>(cli, maxtri);

        triangles_loaded = false;
        path += "/collision_tri.cl";
        k_collision_tri = Kernel(cli, path, "collision_triangle");
        std::cout << "Load collision triganle kernel" << std::endl;
    } 

    //TODO: avoid need for this function?
    void SPH::loadTriangles(std::vector<Triangle> &triangles)
    {
        collision_tri.loadTriangles(triangles);
    }

    void CollisionTriangle::loadTriangles(std::vector<Triangle> &triangles)
    {
        int n_triangles = triangles.size();
        cl_triangles.copyToDevice(triangles);

        k_collision_tri.setArg(3, cl_triangles.getDevicePtr());     //triangles
        k_collision_tri.setArg(4, n_triangles);                     //number of triangles

        size_t max_loc_memory = 1024 << 4;  // 16k bytes local memory on mac
        int max_tri = max_loc_memory / sizeof(Triangle);
        max_tri = 220; // fits in cache

        size_t sz = max_tri*sizeof(Triangle);

        k_collision_tri.setArgShared(7, sz);
        triangles_loaded = true;
    }

    void CollisionTriangle::execute(int num,
                                    float dt,
                                    //input
                                    //Buffer<float4>& svars, 
                                    Buffer<float4>& pos_s, 
                                    Buffer<float4>& vel_s, 
                                    Buffer<float4>& force_s, 
                                    //output
                                    //params
                                    Buffer<SPHParams>& sphp,
                                    //debug
                                    Buffer<float4>& clf_debug,
                                    Buffer<int4>& cli_debug)
    {
    
    int local_size = 32;
    //printf("triangles loaded? %d\n", triangles_loaded);
    if(triangles_loaded)
    {
        
        //k_collision_tri.setArg(0, svars.getDevicePtr());
        k_collision_tri.setArg(0, pos_s.getDevicePtr());
        k_collision_tri.setArg(1, vel_s.getDevicePtr());
        k_collision_tri.setArg(2, force_s.getDevicePtr());
        // 1 = triangles
        // 2 = n_triangles
        k_collision_tri.setArg(5, dt);
        k_collision_tri.setArg(6, sphp.getDevicePtr());
        // 5 = local triangles
        // ONLY IF DEBUGGING
        k_collision_tri.setArg(8, clf_debug.getDevicePtr());
        k_collision_tri.setArg(9, cli_debug.getDevicePtr());


        //printf("execute!\n");
        float gputime = k_collision_tri.execute(num, local_size);
    }
}

}
