
#ifndef RTPS_SPH_H_INCLUDED
#define RTPS_SPH_H_INCLUDED

#include <cmath>
#include <string>
#include <QOpenGLFunctions_4_3_Core>

#include <RTPS.h>
#include <System.h>
#include <Kernel.h>
#include <Buffer.h>

#include <Domain.h>
#include <SPHSettings.h>

#include <util.h>


#include <Hash.h>
#include <BitonicSort.h>
#include <Radix.h>
#include <CellIndices.h>
#include <Permute.h> // contains CloudPermute
#include <sph/Density.h>
#include <sph/Force.h>
#include <sph/Collision_wall.h>
#include <sph/Collision_triangle.h>
#include <sph/LeapFrog.h>
#include <sph/Lifetime.h>
#include <sph/Euler.h>

#include <Hose.h>

#include <timer_eb.h>

#ifdef WIN32
    #if defined(rtps_EXPORTS)
        #define RTPS_EXPORT __declspec(dllexport)
    #else
        #define RTPS_EXPORT __declspec(dllimport)
	#endif 
#else
    #define RTPS_EXPORT
#endif

namespace rtps
{
    using namespace sph;
    class RTPSettings;

    class RTPS_EXPORT SPH : public System
    {
    public:
        SPH(RTPS *ps, int num, int nb_in_cloud=0);
        ~SPH();

        void update();
        int addBox(int nn, float4 min, float4 max, bool scaled, float4 color=float4(1.0f, 0.0f, 0.0f, 1.0f));
        void addBall(int nn, float4 center, float radius, bool scaled);

        int addHose(int total_n, float4 center, float4 velocity, float radius, float4 color=float4(1.0, 0.0, 0.0, 1.0f));
        void updateHose(int index, float4 center, float4 velocity, float radius, float4 color=float4(1.0, 0.0, 0.0, 1.0f));
        void refillHose(int index, int refill);
        void sprayHoses();


        void loadTriangles(std::vector<Triangle> &triangles);

        void testDelete();
        int cut; //for debugging DEBUG

        EB::TimerList timers;
        int setupTimers();
        void printTimers();
        void pushParticles(vector<float4> pos, float4 velo, float4 color=float4(1.0, 0.0, 0.0, 1.0));
        void pushParticles(vector<float4> pos, vector<float4> velo, float4 color=float4(1.0, 0.0, 0.0, 1.0));

        std::vector<float4> getDeletedPos();
        std::vector<float4> getDeletedVel();

    private:
        RTPS* ps;
        RTPSettings* settings;

        SPHParams sphp;
        GridParams grid_params;
        GridParams grid_params_scaled;
        Integrator integrator;
        float spacing; //Particle rest distance in world coordinates

        std::string sph_source_dir;
        int nb_var;

        std::vector<float4> deleted_pos;
        std::vector<float4> deleted_vel;


        //keep track of hoses
        std::vector<Hose*> hoses;

        //needs to be called when particles are added
        void calculateSPHSettings();
        void setupDomain();
        void prepareSorted();

        std::vector<float4> positions;
        std::vector<float4> colors;
        std::vector<float4> velocities;
        std::vector<float4> veleval;

        std::vector<float>  densities;
        std::vector<float4> forces;
        std::vector<float4> xsphs;

        Buffer<float4>      cl_position_u;
        Buffer<float4>      cl_position_s;
        Buffer<float4>      cl_color_u;
        Buffer<float4>      cl_color_s;
        Buffer<float4>      cl_velocity_u;
        Buffer<float4>      cl_velocity_s;
        Buffer<float4>      cl_veleval_u;
        Buffer<float4>      cl_veleval_s;

        Buffer<float>       cl_density_s;
        Buffer<float4>      cl_force_s;
        Buffer<float4>      cl_xsph_s;

        Buffer<unsigned int>    cl_cell_indices_start;
        Buffer<unsigned int>    cl_cell_indices_end;
        Buffer<unsigned int>    cl_sort_hashes;
        Buffer<unsigned int>    cl_sort_indices;

        //should be moved to within bitonic
        Buffer<unsigned int>         cl_sort_output_hashes;
        Buffer<unsigned int>         cl_sort_output_indices;

        Bitonic<unsigned int> bitonic;
        Radix<unsigned int> radix;

        //Parameter structs
        Buffer<SPHParams>   cl_sphp;
        Buffer<GridParams>  cl_GridParams;
        Buffer<GridParams>  cl_GridParamsScaled;

        Buffer<float4>      clf_debug;  //just for debugging cl files
        Buffer<int4>        cli_debug;  //just for debugging cl files

        //CPU functions
        void cpuDensity();
        void cpuPressure();
        void cpuViscosity();
        void cpuXSPH();
        void cpuCollision_wall();
        void cpuCollision_cloud();
        void cpuEuler();
        void cpuLeapFrog();

        //calculate the various parameters that depend on max_num of particles
        void calculate();
        //copy the SPH parameter struct to the GPU
        void updateSPHP();

        //Nearest Neighbors search related functions
        //Prep prep;
        void call_prep(int stage);
        Hash hash;
        //DataStructures datastructures;
        CellIndices cellindices;
        Permute permute;
        void hash_and_sort();
        void cloud_hash_and_sort();  // GE
        void bitonic_sort();
        void radix_sort();
        void cloud_bitonic_sort();   // GE
        Density density;
        Force force;
        void collision();
        CollisionWall collision_wall;
        CollisionTriangle collision_tri;
        //CollisionCloud collision_cloud;
        void integrate();
        LeapFrog leapfrog;
        Euler euler;
        //CloudEuler cloud_euler;

        Lifetime lifetime;

        float Wpoly6(float4 r, float h);
        float Wspiky(float4 r, float h);
        float Wviscosity(float4 r, float h);

		Utils u;
    };



};

#endif
