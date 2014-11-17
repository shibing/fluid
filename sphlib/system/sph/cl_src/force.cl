

#ifndef _NEIGHBORS_CL_
#define _NEIGHBORS_CL_


#define ARGS __global float4* pos, __global float* density, __global float* mass, __global float* rest_density,  __global float4* veleval, __global float4* force, __global float4* xsph
#define ARGV pos, density, mass, rest_density, veleval, force, xsph

/*----------------------------------------------------------------------*/

#include "cl_macros.h"
#include "cl_structs.h"
#include "cl_kernels.h"


//----------------------------------------------------------------------
inline void ForNeighbor(
                        ARGS,
                        PointData* pt,
                        uint index_i,
                        uint index_j,
                        float4 position_i,
                        __constant struct GridParams* gp,
                        __constant struct SPHParams* sphp
                        DEBUG_ARGS
                       )
{
    float4 position_j = pos[index_j] * sphp->simulation_scale; 

    float mass_i = mass[index_i];
    float mass_j = mass[index_j];
    float rest_density_i = rest_density[index_i];
    float rest_density_j = rest_density[index_j];
    float delta_i = density[index_i];
    float delta_j = density[index_j];
    float inv_delta_i = 1 / delta_i;
    float inv_delta_j = 1 / delta_j;
    float density_i = mass_i * density[index_i];
    float density_j = mass_j * density[index_j];
    float inv_density_i = 1 / density_i;
    float inv_density_j = 1 / density_j;

    float4 r = (position_i - position_j); 
    r.w = 0.f;
    float rlen = length(r);

    if (rlen <= sphp->smoothing_distance)
    {
        int iej = index_i != index_j;

        rlen = max(rlen, sphp->EPSILON);

        float dWijdr = Wspiky_dr(rlen, sphp->smoothing_distance, sphp);


        float Pi = sphp->K * (density_i - rest_density_i);
        float Pj = sphp->K * (density_j - rest_density_j);

       // float kern = -.5 * dWijdr * (Pi + Pj) * sphp->wspiky_d_coef * idi * idj;
        float kern = -1.0f * (Pi * inv_delta_i * inv_delta_i + Pj * inv_delta_j * inv_delta_j) * sphp->wspiky_d_coef * dWijdr;
        float4 force = kern * r;

        // Add viscous forces
        float4 veli = veleval[index_i]; 
        float4 velj = veleval[index_j];
        float vvisc = sphp->viscosity;
        float dWijlapl = sphp->wvisc_dd_coef * Wvisc_lapl(rlen, sphp->smoothing_distance, sphp);
        float4 visc = vvisc * (velj-veli) * dWijlapl * inv_delta_i * inv_delta_j;

        force += visc;

        force /= mass_i;

        float Wijpol6 = Wpoly6(r, sphp->smoothing_distance, sphp);
        float4 xsph = (2.f * mass_i * Wijpol6 * (velj - veli)/(inv_density_i + inv_density_j));
        pt->xsph += xsph * (float)iej;
        pt->xsph.w = 0.f;
        pt->force += force * (float)iej;

    }
}

#include "cl_neighbors.h"

__kernel void force_update(
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
    force[index] = pt.force; 
    clf[index].xyz = pt.force.xyz;
    xsph[index] = sphp->wpoly6_coef * pt.xsph;
}
#endif

