
#include "cl_macros.h"
#include "cl_structs.h"


__kernel void leapfrog(
                      __global float4* pos_u,
                      __global float4* pos_s,
                      __global float4* vel_u,
                      __global float4* vel_s,
                      __global float4* veleval_u,
                      __global float4* force_s,
                      __global float4* xsph_s,
                      __global float4* color_u,
                      __global float4* color_s,
                      __constant struct GridParams* gp,
                      __constant struct SPHParams* sphp, 
                      float dt)
{
    unsigned int i = get_global_id(0);
    int num = sphp->num;
    if (i >= num) return;

    float4 p = pos_s[i] * sphp->simulation_scale;
    float4 v = vel_s[i];
    float4 f = force_s[i];

    float speed = length(f);
    if (speed > sphp->velocity_limit) {
        f *= sphp->velocity_limit/speed;
    }

    float4 vnext = v + dt*f;
    vnext.w = 0;
    vnext += sphp->xsph_factor * xsph_s[i];
    p += dt * vnext;
    p.w = 1.0f; 

    float4 veval = 0.5f * (v + vnext);
    p.xyz /= sphp->simulation_scale;

    vel_u[i] = vnext;
    veleval_u[i] = veval; 
    pos_u[i] = (float4)(p.xyz, 1.0f);  
    color_u[i] = color_s[i];

    //碰撞检测与处理

    /* float boundary_distance = sphp->rest_distance * 0.5f; */
    /* float d = 0; */
    /* float C = 1.0; */
    /* float4 normal; */
    /* float l = dt * length(vnext); */
    /* float damp = -0; */

    /* if(p.x < gp->bnd_min.x) { */
    /*     d = gp->bnd_min.x - p.x; */
    /*     p.x = gp->bnd_min.x; */
    /*     normal = (float4)(1, 0, 0, 0); */
    /*     vnext.x = vnext.x - (1 + C * d / (dt * vnext.x)) *dot(vnext, normal); */ 
    /* } */
    /* if(p.x > gp->bnd_max.x) { */
    /*     d = p.x - gp->bnd_max.x; */
    /*     p.x = gp->bnd_max.x; */
    /*     normal = (float4)(-1, 0, 0, 0); */
    /*     vnext.x = vnext.x + (1 + C * d / (dt * vnext.x)) *dot(vnext, normal); */ 
    /* } */
    /* //y-direction */
    /* if(p.y < gp->bnd_min.y) { */
    /*     d = gp->bnd_min.y - (p.y); */ 
    /*     p.y = gp->bnd_min.y; */
    /*     normal = (float4)(0, 1, 0, 0); */
    /*     vnext.y = vnext.y - (1 + C * d / (dt * vnext.y)) * dot(vnext, normal); */ 
    /* } */
    /* if(p.y >  gp->bnd_max.y) { */
    /*     d = p.y - gp->bnd_max.y; */
    /*     p.y = gp->bnd_max.y; */
    /*     normal = (float4)(0, -1, 0, 0); */
    /*     vnext.y = vnext.y + (1 + C * d / (dt * vnext.y)) * dot(vnext, normal); */ 
    /* } */

    /* /1* //z-direction *1/ */
    /* if(p.z < gp->bnd_min.z) { */
    /*     d = gp->bnd_min.z - (p.z); */ 
    /*     p.z = gp->bnd_min.z; */
    /*     normal = (float4)(0, 0, 1, 0); */
    /*     vnext.z = vnext.z - (1 + C * d / (dt * vnext.z)) * dot(vnext, normal); */ 
    /* } */
    /* if(p.z  > gp->bnd_max.z) { */
    /*     d = p.z - gp->bnd_max.z; */
    /*     p.z = gp->bnd_max.z ; */
    /*     normal = (float4)(0, 0, -1, 0); */
    /*     vnext.z = vnext.z + (1 + C * d / (dt * vnext.z)) * dot(vnext, normal); */ 
    /* } */

}


