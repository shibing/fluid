
#include "cl_macros.h"
#include "cl_structs.h"


__kernel void leapfrog(
                      __global float4* pos_u,
                      __global float4* pos_s,
                      __global float4* vel_u,
                      __global float4* vel_s,

                      __global float* mass_u,
                      __global float* mass_s,
                      __global float* rest_density_u,
                      __global float* rest_density_s,

                      __global float4* color_u,
                      __global float4* color_s,

                      __global float4* veleval_u,
                      __global float4* force_s,
                      __global float4* xsph_s,
                      
                      __constant struct SPHParams* sphp, 
                      float dt)
{
    unsigned int i = get_global_id(0);
    int num = sphp->num;
    if (i >= num) return;

    float4 p = pos_s[i] * sphp->simulation_scale;
    float4 v = vel_s[i];
    float4 f = force_s[i];

    f.y += sphp->gravity;
    f.w = 0.f;

    float speed = length(f);
    if (speed > sphp->velocity_limit) {
        f *= sphp->velocity_limit/speed;
    }

    float4 vnext = v + dt*f;
    vnext += sphp->xsph_factor * xsph_s[i];

    /* float4 veval = 0.5f*(v+vnext); */

    flaot4 veval;
    veval = vnext + 0.5 * dt * f;

    p += dt * vnext;
    p.w = 1.0f; 
    p.xyz /= sphp->simulation_scale;

    vel_u[i] = vnext;
    veleval_u[i] = veval; 
    pos_u[i] = (float4)(p.xyz, 1.0f);  

    mass_u[i] = mass_s[i];
    rest_density_u[i] = rest_density_s[i];

    color_u[i] = color_s[i];
}


