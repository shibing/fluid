
#ifndef _NEIGHBORS_CL_
#define _NEIGHBORS_CL_


#define ARGS __global float4* pos, __global float* density, __global float4 * normal_v
#define ARGV pos, density, normal_v


/*----------------------------------------------------------------------*/

#include "cl_macros.h"
#include "cl_structs.h"
#include "cl_kernels.h"


//----------------------------------------------------------------------
inline void ForNeighbor(ARGS,
                        PointData* pt,
                        uint index_i,
                        uint index_j,
                        float4 position_i,
                        __constant struct GridParams* gp,
                        __constant struct SPHParams* sphp
                        DEBUG_ARGS
                       )
{
    int num = sphp->num;

    float4 position_j = pos[index_j] * sphp->simulation_scale; 
    float4 r = (position_i - position_j); 
    r.w = 0.f; 
    float rlen = length(r);
    float beta = .06;

    if (rlen <= sphp->smoothing_distance) {
        
        float dWijdr = Wpoly6_dr(rlen, sphp->smoothing_distance, sphp);
        //dWijdr = Wspiky_dr(rlen, sphp->smoothing_distance, sphp);
        float mag = dWijdr * sphp->wpoly6_d_coef;

        float4 normal = sphp->mass / density[index_j] * mag * r; 
        pt->normal += beta * normal;
    }
}

#include "cl_neighbors.h"


__kernel void compute_normal(
                       ARGS,
                       __global int*    cell_indexes_start,
                       __global int*    cell_indexes_end,
                       __constant struct GridParams* gp,
                       __constant struct SPHParams* sphp 
                       DEBUG_ARGS
                       )
{
    int nb_vars = sphp->nb_vars;
    int num = sphp->num;

    int index = get_global_id(0);
    if (index >= num) return;

    float4 position_i = pos[index] * sphp->simulation_scale;

    clf[index] = (float4)(99,0,0,0);

    PointData pt;
    zeroPoint(&pt);

    IterateParticlesInNearbyCells(ARGV, &pt, num, index, position_i, cell_indexes_start, cell_indexes_end, gp, sphp DEBUG_ARGV);
    normal_v[index] = pt.normal;
    clf[index].xyz = pt.normal.xyz;
}

#endif

