
#ifndef _NEIGHBORS_CL_
#define _NEIGHBORS_CL_


#define ARGS __global float4* pos, __global float* density
#define ARGV pos, density


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

    if (rlen <= sphp->smoothing_distance)
    {
        float Wij = Wpoly6(r, sphp->smoothing_distance, sphp);

        pt->density.x += sphp->mass*Wij;
    }
}

#include "cl_neighbors.h"


__kernel void density_update(
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
    density[index] = sphp->wpoly6_coef * pt.density.x;
    clf[index].w = density[index];
}

#endif

