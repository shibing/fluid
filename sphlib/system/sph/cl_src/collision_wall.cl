
#include "cl_macros.h"
#include "cl_structs.h"
#include "cl_collision.h"

__kernel void collision_wall(
                            int push,
                            __global float4* pos_s,
                            __global float4* vel_s,
                            __global float4* force_s,
                            __constant struct GridParams* gp,
                            __constant struct SPHParams* sphp)
{
    unsigned int i = get_global_id(0);
    int num = sphp->num;
    if (i >= num) return;
    if(pos_s[i].w < 0)
        return;

    float bound_dist = sphp->rest_distance * .5f;


    float4 p = pos_s[i] * sphp->simulation_scale;
    float4 v = vel_s[i];
    float4 f = force_s[i];

    float4 r_f = (float4)(0.f, 0.f, 0.f, 0.f);
    float4 f_f = (float4)(0.f, 0.f, 0.f, 0.f);

    float friction_kinetic = 0.0f;
    float friction_static_limit = 0.0f;

    //Z walls
    float diff = bound_dist - (p.z - gp->bnd_min.z);
    if (diff > sphp->EPSILON)
    {
        float4 normal = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
        r_f += calculateRepulsionForce(normal, v, sphp->boundary_stiffness, sphp->boundary_dampening, diff);
        f_f += calculateFrictionForce(v, f, normal, friction_kinetic, friction_static_limit);
    }
    diff = bound_dist - (gp->bnd_max.z - p.z);
    if (diff > sphp->EPSILON)
    {
        float4 normal = (float4)(0.0f, 0.0f, -1.0f, 0.0f);
        r_f += calculateRepulsionForce(normal, v, sphp->boundary_stiffness, sphp->boundary_dampening, diff);
        f_f += calculateFrictionForce(v, f, normal, friction_kinetic, friction_static_limit);
    }

    //Y walls
    diff = bound_dist - (p.y - gp->bnd_min.y);
    if (diff > sphp->EPSILON)
    {
        float4 normal = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
        r_f += calculateRepulsionForce(normal, v, sphp->boundary_stiffness, sphp->boundary_dampening, diff);
        f_f += calculateFrictionForce(v, f, normal, friction_kinetic, friction_static_limit);
    }
    diff = bound_dist - (gp->bnd_max.y - p.y);
    if (diff > sphp->EPSILON)
    {
        float4 normal = (float4)(0.0f, -1.0f, 0.0f, 0.0f);
        r_f += calculateRepulsionForce(normal, v, sphp->boundary_stiffness, sphp->boundary_dampening, diff);
        f_f += calculateFrictionForce(v, f, normal, friction_kinetic, friction_static_limit);
    }
    //X walls
    if(push == 1)
        diff = bound_dist - (p.x - gp->bnd_min.x - 0.06);
    else
        diff = bound_dist - (p.x - gp->bnd_min.x);

    if (diff > sphp->EPSILON)
    {
        float4 normal = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
        r_f += calculateRepulsionForce(normal, v, sphp->boundary_stiffness, sphp->boundary_dampening, diff);
        f_f += (float4)(1.0, 0.0, 0.0, 0.0);
        f_f += calculateFrictionForce(v, f, normal, friction_kinetic, friction_static_limit);
    }
    diff = bound_dist - (gp->bnd_max.x - p.x);
    if (diff > sphp->EPSILON)
    {
        float4 normal = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
        r_f += calculateRepulsionForce(normal, v, sphp->boundary_stiffness, sphp->boundary_dampening, diff);
        f_f += calculateFrictionForce(v, f, normal, friction_kinetic, friction_static_limit);
    }
    force_s[i] += r_f + f_f;

}
