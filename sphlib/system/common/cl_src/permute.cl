// This software contains source code provided by NVIDIA Corporation.
// Specifically code from the CUDA 2.3 SDK "Particles" sample

#ifndef _PERMUTE_
#define _PERMUTE_

#include "cl_macros.h"
#include "cl_structs.h"

__kernel void permute(
                            int num,
                            __global float4* pos_u,
                            __global float4* pos_s,
                            __global float4* vel_u,
                            __global float4* vel_s,
                            __global float*  mass_u,
                            __global float*  mass_s,
                            __global float*  rest_density_u,
                            __global float*  rest_density_s,
                            __global float4* veleval_u,
                            __global float4* veleval_s,
                            __global float4* color_u,
                            __global float4* color_s,
                            __global uint*   sort_indices
                            )
{
    uint index = get_global_id(0);
    if (index >= num) return;
    uint sorted_index = sort_indices[index];
    pos_s[index]     = pos_u[sorted_index];
    vel_s[index]     = vel_u[sorted_index];
    mass_s[index]    = mass_u[sorted_index];
    rest_density_s[index]  = rest_density_u[sorted_index];
    veleval_s[index] = veleval_u[sorted_index]; 
    color_s[index]   = color_u[sorted_index];
}

#endif
