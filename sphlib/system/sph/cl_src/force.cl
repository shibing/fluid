

#ifndef _NEIGHBORS_CL_
#define _NEIGHBORS_CL_


#define ARGS __global float4* pos, __global float* density, __global float4* veleval, __global float4* normal_v, __global float4* force, __global float4* xsph
#define ARGV pos, density, veleval, normal_v, force, xsph

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
    if(position_j.w < 0)
        return;
    float4 r = (position_i - position_j); 
    r.w = 0.f;
    float rlen = length(r);

    if (rlen > 0.000001 && rlen <= sphp->smoothing_distance)
    {

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

         Pi =  max(0.0, sphp->K * rest_density / 7.0 * (rhoi_rho0 - 1));
         Pj =  max(0.0, sphp->K * rest_density/ 7.0 * (rhoj_rho0 - 1)); 

         /* Pi =   sphp->K * rest_density / 7.0 * (rhoi_rho0 - 1); */
         /* Pj =   sphp->K * rest_density/ 7.0 * (rhoj_rho0 - 1); */ 

        /* Pi = max(0.0f, sphp->K*(di - rest_density)); */
        /* Pj = max(0.0f, sphp->K*(dj - rest_density)); */

       // float kern = -.5 * dWijdr * (Pi + Pj) * sphp->wspiky_d_coef * idi * idj;
        float mag = -1.0f * dWijdr * (Pi * idi * idi + Pj * idj * idj) * sphp->wspiky_d_coef;
        float4 force = (mag + 0) * r; 

        float4 veli = veleval[index_i]; 
        float4 velj = veleval[index_j];

        // Add viscous forces
        float vvisc = sphp->viscosity;
        float dWijlapl = sphp->wvisc_dd_coef * Wvisc_lapl(rlen, sphp->smoothing_distance, sphp);
        float4 visc = vvisc * (velj-veli) * dWijlapl * idj * idi;
        force += visc;
        force *= sphp->mass;
        pt->force += force;

        //surface tension
        float gama = 0.01f;
        float4 st_force;
        st_force = -gama * sphp->mass  * sphp->wspline_coef * Wspline(rlen, sphp->smoothing_distance, sphp) / rlen * r;

        float4 curvature_force = -gama * (normal_v[index_i] - normal_v[index_j]);

        float K_ij = 2 * sphp->rest_density / (di + dj);

        pt->force += K_ij * (1.5 * st_force + curvature_force);

        float Wijpol6 = Wpoly6(r, sphp->smoothing_distance, sphp);
        float4 xsph = (2.f * sphp->mass * Wijpol6 * (velj-veli)/(di+dj));
        pt->xsph += xsph;
        pt->xsph.w = 0.f;

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
    if(position_i.w < 0)
        return;

    clf[index] = (float4)(99,0,0,0);

    PointData pt;
    zeroPoint(&pt);

    IterateParticlesInNearbyCells(ARGV, &pt, num, index, position_i, cell_indexes_start, cell_indexes_end, gp, sphp DEBUG_ARGV);
    pt.force.y += sphp->gravity;
    pt.force.w = 0;
    force[index] = pt.force; 

    clf[index].xyz = pt.force.xyz;
    xsph[index] = sphp->wpoly6_coef * pt.xsph;
}
#endif

