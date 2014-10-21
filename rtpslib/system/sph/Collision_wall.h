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


#ifndef RTPS_COLLISION_WALL_H_INCLUDED
#define RTPS_COLLISION_WALL_H_INCLUDED


#include <CLL.h>
#include <Buffer.h>


namespace rtps
{
    class CollisionWall
    {
        public:
            CollisionWall() { cli = NULL; timer = NULL; };
            CollisionWall(std::string path, CL* cli, EB::Timer* timer);
            void execute(int num,
                        Buffer<float4>& pos_s, 
                        Buffer<float4>& vel_s, 
                        Buffer<float4>& force_s, 
                        //Buffer<float4>& svars, 
                        //params
                        Buffer<SPHParams>& sphp,
                        Buffer<GridParams>& gp,
                        //debug
                        Buffer<float4>& clf_debug,
                        Buffer<int4>& cli_debug);
            
           

        private:
            CL* cli;
            Kernel k_collision_wall;
            EB::Timer* timer;
    };
}

#endif
