

#ifndef _NEIGHBORS_CL_
#define _NEIGHBORS_CL_


#define ARGS __global float4* pos, __global float* density, __global float4* veleval, __global float4* force, __global float4* xsph
#define ARGV pos, density, veleval, force, xsph

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
    int num = sphp->num;

    float4 position_j = pos[index_j] * sphp->simulation_scale; 
    float4 r = (position_i - position_j); 
    r.w = 0.f;
    float rlen = length(r);

    if (rlen <= sphp->smoothing_distance)
    {
        int iej = index_i != index_j;

        rlen = max(rlen, sphp->EPSILON);

        float dWijdr = Wspiky_dr(rlen, sphp->smoothing_distance, sphp);

        float di = density[index_i];  
        float dj = density[index_j];
        float idi = 1.0/di;
        float idj = 1.0/dj;


        float rest_density = sphp->rest_density;
		float Pi;
		float Pj;
		
		float rhoi_rho0 = di / rest_density; 
        float rhoj_rho0 = dj / rest_density; 

        rhoi_rho0 = rhoi_rho0 * rhoi_rho0 * rhoi_rho0 * rhoi_rho0 * rhoi_rho0 * rhoi_rho0 * rhoi_rho0 ; 
        rhoj_rho0 = rhoj_rho0 * rhoj_rho0 * rhoj_rho0 * rhoj_rho0 * rhoj_rho0 * rhoj_rho0 * rhoj_rho0 ; 

         Pi = max(0.0f, sphp->K * rest_density / 7.0 * (rhoi_rho0 - 1));
         Pj = max(0.0f, sphp->K * rest_density/ 7.0 * (rhoj_rho0 - 1)); 
		
        //Pi = sphp->K*(di - rest_density);
        //Pj = sphp->K*(dj - rest_density);


       // float kern = -.5 * dWijdr * (Pi + Pj) * sphp->wspiky_d_coef * idi * idj;
        float kern = -1.0f * dWijdr * (Pi * idi * idi + Pj * idj * idj) * sphp->wspiky_d_coef;
        float4 force = kern*r; 

        float4 veli = veleval[index_i]; 
        float4 velj = veleval[index_j];

        // Add viscous forces
        float vvisc = sphp->viscosity;
        float dWijlapl = sphp->wvisc_dd_coef * Wvisc_lapl(rlen, sphp->smoothing_distance, sphp);
        float4 visc = vvisc * (velj-veli) * dWijlapl * idj * idi;
        force += visc;

        //add buoyancy
      //  force += 10.0 * (di - sphp->rest_density) * (0.0, -9.8, 0.0, 1.0) * idi;

        force *= sphp->mass;

        float Wijpol6 = Wpoly6(r, sphp->smoothing_distance, sphp);
        float4 xsph = (2.f * sphp->mass * Wijpol6 * (velj-veli)/(di+dj));
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

