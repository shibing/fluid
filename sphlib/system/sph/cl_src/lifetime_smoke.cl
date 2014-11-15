
#include "cl_macros.h"
#include "cl_structs.h"

__kernel void lifetime( int num,
                        float increment,
                        __global float4* pos_u, 
                        __global float4* color_u, 
                        __global float4* color_s, 
                        __global uint* sort_indices
                        DEBUG_ARGS
                        ) 
{
    unsigned int i = get_global_id(0);
    if(i >= num) return;

    float life = color_s[i].w;
    life += increment;
    if(life <= 0.f)
    {
        life = 0.f;
    }
    if(life >= 3.14)
    {
        life = 3.14f;
        pos_u[i] = (float4)(100.0f, 100.0f, 100.0f, 1.0f);
    }
    float alpha = sin(life);
    
    color_s[i].x = alpha;
    color_s[i].y = alpha;
    color_s[i].z = alpha;
    color_s[i].w = life;

    color_u[i] = color_s[i];



}

