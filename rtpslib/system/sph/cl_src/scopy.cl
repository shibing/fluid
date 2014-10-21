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




__kernel void scopy(int n, 
                    __global float* sxorig, 
                    __global float* sydest)
{
    int i, tid, totalThreads, ctaStart;

    tid = get_local_id(0);
    int locsiz = get_local_size(0);
    int gridDimx = get_num_groups(0);
    int gid = get_group_id(0);

    totalThreads = gridDimx * locsiz;
    ctaStart = locsiz*gid; 

    for (i = ctaStart + tid; i < n; i += totalThreads)
    {
        sydest[i] = sxorig[i];
    }
}
//----------------------------------------------------------------------
