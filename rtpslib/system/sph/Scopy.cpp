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


#include "SPH.h"

#include <string>

namespace rtps
{

    void SPH::loadScopy()
    {
        printf("create scopy kernel\n");
        std::string path(SPH_CL_SOURCE_DIR);
        path = path + "/scopy.cl";
        k_scopy = Kernel(ps->cli, path, "scopy");

    }

    void SPH::scopy(int n, cl_mem xsrc, cl_mem ydst)
    {
        int args = 0;
        k_scopy.setArg(args++, n);
        k_scopy.setArg(args++, xsrc);
        k_scopy.setArg(args++, ydst);

        size_t global = (size_t) n;
        size_t local = 128; //cl.getMaxWorkSize(kern.getKernel());

        //not sure why i can't use local
        k_scopy.execute(global, local);
        ps->cli->queue.finish();

    }

}
