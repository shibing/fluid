/****************************************************************************************
* Real-Time Particle System - An OpenCL based Particle system developed to run on modern GPUs. Includes SPH fluid simulations.
* version 1.0, September 14th 2011
* 
* Copyright (C) 2011 Ian Johnson, Andrew Young, Gordon Erlebacher, Myrna Merced, Evan Bollig
* 
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
****************************************************************************************/


#include "cl_macros.h"
#include "cl_structs.h"
#include "cl_collision.h"



__kernel void collision_wall(
                            //__global float4* vars_sorted, 
                            __global float4* pos_s,
                            __global float4* vel_s,
                            __global float4* force_s,
                            __constant struct GridParams* gp,
                            __constant struct SPHParams* sphp)
{
    unsigned int i = get_global_id(0);
    //int num = get_global_size(0);
    int num = sphp->num;
    if (i > num) return;

    float bound_dist = sphp->rest_distance * .5f;


    /*
    float4 p = pos(i);
    float4 v = vel(i);// * sphp->simulation_scale;
    float4 f = force(i);
    */
    float4 p = pos_s[i] * sphp->simulation_scale;
    float4 v = vel_s[i];// * sphp->simulation_scale;
    float4 f = force_s[i];

    float4 r_f = (float4)(0.f, 0.f, 0.f, 0.f);
    float4 f_f = (float4)(0.f, 0.f, 0.f, 0.f);

    //these should be moved to the sphp struct
    //but set to 0 in both of Krog's simulations...
    float friction_kinetic = 0.0f;
    float friction_static_limit = 0.0f;

    //Z walls
    float diff = bound_dist - (p.z - gp->bnd_min.z);
    if (diff > sphp->EPSILON)
    {
        float4 normal = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
        r_f += calculateRepulsionForce(normal, v, sphp->boundary_stiffness, sphp->boundary_dampening, diff);
        f_f += calculateFrictionForce(v, f, normal, friction_kinetic, friction_static_limit);
        //r_f += calculateRepulsionForce(normal, v, boundary_stiffness, boundary_dampening, boundary_distance);
    }
    diff = bound_dist - (gp->bnd_max.z - p.z);
    if (diff > sphp->EPSILON)
    {
        float4 normal = (float4)(0.0f, 0.0f, -1.0f, 0.0f);
        r_f += calculateRepulsionForce(normal, v, sphp->boundary_stiffness, sphp->boundary_dampening, diff);
        f_f += calculateFrictionForce(v, f, normal, friction_kinetic, friction_static_limit);
        //r_f += calculateRepulsionForce(normal, v, boundary_stiffness, boundary_dampening, boundary_distance);
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
    diff = bound_dist - (p.x - gp->bnd_min.x);
    if (diff > sphp->EPSILON)
    {
        float4 normal = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
        r_f += calculateRepulsionForce(normal, v, sphp->boundary_stiffness, sphp->boundary_dampening, diff);
        f_f += calculateFrictionForce(v, f, normal, friction_kinetic, friction_static_limit);
    }
    diff = bound_dist - (gp->bnd_max.x - p.x);
    if (diff > sphp->EPSILON)
    {
        float4 normal = (float4)(-1.0f, 0.0f, 0.0f, 0.0f);
        r_f += calculateRepulsionForce(normal, v, sphp->boundary_stiffness, sphp->boundary_dampening, diff);
        f_f += calculateFrictionForce(v, f, normal, friction_kinetic, friction_static_limit);
    }


    //force(i) += r_f + f_f;
    force_s[i] += r_f + f_f;

}
