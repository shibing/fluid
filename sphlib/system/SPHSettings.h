
#ifndef RTPS_SPHSETTINGS_H_INCLUDED
#define RTPS_SPHSETTINGS_H_INCLUDED

#include <stdlib.h>
#include <string>
#include <map>
#include <iostream>
#include <stdio.h>
#include <sstream>

#include <structs.h>
#include <Buffer.h>
#include <Domain.h>

namespace rtps
{
    //Struct which gets passed to OpenCL routines
	struct __attribute__((aligned(16)))  SPHParams
    {
        float mass;
        float rest_density;
        float rest_distance;
        float smoothing_distance;
        float simulation_scale;
        
        float boundary_stiffness;
        float boundary_dampening;
        float boundary_distance;
        float K;        //gas constant
        
        float viscosity;
        float velocity_limit;
        float xsph_factor;
        float gravity; // -9.8 m/sec^2

        float friction_coef;
        //next 4 not used at the moment
        float restitution_coef;
        float shear;
        float attraction;

        float spring;
        //float surface_threshold;
        //constants
        float EPSILON;
        float PI;       //delicious
        //Kernel Coefficients
        float wpoly6_coef;
        
        float wpoly6_d_coef;
        float wpoly6_dd_coef; // laplacian
        float wspiky_coef;
        float wspiky_d_coef;

        float wspiky_dd_coef;
        float wvisc_coef;
        float wvisc_d_coef;
        float wvisc_dd_coef;
        float wspline_coef;

        //CL parameters
        int num;
        int nb_vars; // for combined variables (vars_sorted, etc.)
        int choice; // which kind of calculation to invoke
        int max_num;

		//CL parameter, cloud
        int cloud_num; // nb cloud points
        int max_cloud_num;

        void print()
        {
            printf("----- SPHParams ----\n");
            printf("mass: %f\n", mass);
            printf("rest distance: %f\n", rest_distance);
            printf("smoothing distance: %f\n", smoothing_distance);
            printf("simulation_scale: %f\n", simulation_scale);
            printf("--------------------\n");
        }
    };
    enum Integrator
    {
        EULER, LEAPFROG
    };

}

#endif
