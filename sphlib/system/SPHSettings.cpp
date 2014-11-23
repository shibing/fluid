
#include <SPH.h>
#include <RTPSettings.h>

namespace rtps
{
    void SPH::calculate()
    {
        float rho0 = 1000;                              //rest density [kg/m^3 ]
        float VP = 2 * .0262144 / 131072;              //Particle Volume [ m^3 ]
        float mass = rho0 * VP;                         //Particle Mass [ kg ]
        float rest_distance =  pow(VP, 1.f/3.f);   //rest distance between particles [ m ]
        float smoothing_distance = 2.0001f * rest_distance;//interaction radius

        float4 dmin = grid->getBndMin();
        float4 dmax = grid->getBndMax();
        float domain_vol = (dmax.x - dmin.x) * (dmax.y - dmin.y) * (dmax.z - dmin.z);

        float simulation_scale = pow(0.5 * VP * 131072 / (5 * 5 * 5), 1.f/3.f); 

        settings->SetSetting("Maximum Number of Particles", max_num);
        settings->SetSetting("Mass", mass);
        settings->SetSetting("rho0", rho0);
        settings->SetSetting("Rest Distance", rest_distance);
        settings->SetSetting("Smoothing Distance", smoothing_distance);
        settings->SetSetting("Simulation Scale", simulation_scale);

        float boundary_distance = .5f * rest_distance;

        settings->SetSetting("Boundary Distance", boundary_distance);
        float spacing = rest_distance / simulation_scale;
        settings->SetSetting("Spacing", spacing);
        settings->SetSetting("Spacing_No_Scale", rest_distance);
 
        float pi = M_PI;
        float h9 = pow(smoothing_distance, 9.f);
        float h6 = pow(smoothing_distance, 6.f);
        float h3 = pow(smoothing_distance, 3.f);
        
        //Kernel Coefficients
        settings->SetSetting("wpoly6", 315.f/64.0f/pi/h9 );
        settings->SetSetting("wpoly6_d", -945.f/(32.0f*pi*h9) );  //doesn't seem right
        settings->SetSetting("wpoly6_dd", -945.f/(32.0f*pi*h9) ); // laplacian
        settings->SetSetting("wspiky", 15.f/pi/h6 );
        settings->SetSetting("wspiky_d", -45.f/(pi*h6) );
        settings->SetSetting("wspiky_dd", 15./(2.*pi*h3) );
        settings->SetSetting("wvisc", 15./(2.*pi*h3) );
        settings->SetSetting("wvisc_d", 15./(2.*pi*h3) ); //doesn't seem right
        settings->SetSetting("wvisc_dd", 45./(pi*h6) );
        settings->SetSetting("wspline_coef", 32.0f/(pi * h9));

        //dynamic params
        settings->SetSetting("Gravity", -9.8f); // -9.8 m/sec^2
        settings->SetSetting("Gas Constant", 1000.0f);
        settings->SetSetting("Viscosity", 0.01f);
        settings->SetSetting("Acceleration Limit", 700.0f);
        settings->SetSetting("XSPH Factor", .01f);
        settings->SetSetting("Friction Kinetic", 0.0f);
        settings->SetSetting("Friction Static", 0.0f);
        settings->SetSetting("Boundary Stiffness", 20000.0f);
        settings->SetSetting("Boundary Dampening", 256.0f);

        //next 4 not used at the moment
        settings->SetSetting("Restitution", 0.0f);
        settings->SetSetting("Shear", 0.0f);
        settings->SetSetting("Attraction", 0.0f);
        settings->SetSetting("Spring", 0.0f);

        //constants
        settings->SetSetting("EPSILON", 1E-6);
        settings->SetSetting("PI", M_PI);

        //CL parameters
        settings->SetSetting("Number of Particles", 0);
        settings->SetSetting("Number of Variables", 10); // for combined variables (vars_sorted, etc.) //TO be depracated

    }
   
	//----------------------------------------------------------------------
    void SPH::updateSPHP()
    {
        sphp.mass = settings->GetSettingAs<float>("Mass");
        sphp.rest_density = settings->GetSettingAs<float>("rho0");
        sphp.rest_distance = settings->GetSettingAs<float>("Rest Distance");
        sphp.smoothing_distance = settings->GetSettingAs<float>("Smoothing Distance");
        sphp.simulation_scale = settings->GetSettingAs<float>("Simulation Scale");
        sphp.boundary_stiffness = settings->GetSettingAs<float>("Boundary Stiffness");
        sphp.boundary_dampening = settings->GetSettingAs<float>("Boundary Dampening");
        sphp.boundary_distance = settings->GetSettingAs<float>("Boundary Distance");
        sphp.K = settings->GetSettingAs<float>("Gas Constant");        
        sphp.viscosity = settings->GetSettingAs<float>("Viscosity");
        sphp.velocity_limit = settings->GetSettingAs<float>("Acceleration Limit");
        sphp.xsph_factor = settings->GetSettingAs<float>("XSPH Factor");
        sphp.gravity = settings->GetSettingAs<float>("Gravity"); // -9.8 m/sec^2
        sphp.friction_coef = settings->GetSettingAs<float>("Friction");
        sphp.restitution_coef = settings->GetSettingAs<float>("Restitution");

        //next 3 not used at the moment
        sphp.shear = settings->GetSettingAs<float>("Shear");
        sphp.attraction = settings->GetSettingAs<float>("Attraction");
        sphp.spring = settings->GetSettingAs<float>("Spring");
        //sphp.surface_threshold;

        //constants
        sphp.EPSILON = settings->GetSettingAs<float>("EPSILON");
        sphp.PI = settings->GetSettingAs<float>("PI");       //delicious
        //Kernel Coefficients
        sphp.wpoly6_coef = settings->GetSettingAs<float>("wpoly6");
        sphp.wpoly6_d_coef = settings->GetSettingAs<float>("wpoly6_d");
        sphp.wpoly6_dd_coef = settings->GetSettingAs<float>("wpoly6_dd"); // laplacian
        sphp.wspiky_coef = settings->GetSettingAs<float>("wspiky");
        sphp.wspiky_d_coef = settings->GetSettingAs<float>("wspiky_d");
        sphp.wspiky_dd_coef = settings->GetSettingAs<float>("wspiky_dd");
        sphp.wvisc_coef = settings->GetSettingAs<float>("wvisc");
        sphp.wvisc_d_coef = settings->GetSettingAs<float>("wvisc_d");
        sphp.wvisc_dd_coef = settings->GetSettingAs<float>("wvisc_dd");
        sphp.wspline_coef  = settings->GetSettingAs<float>("wspline_coef");

        //CL parameters
        sphp.num = settings->GetSettingAs<int>("Number of Particles");
        sphp.nb_vars = settings->GetSettingAs<int>("Number of Variables"); // for combined variables (vars_sorted, etc.)
        sphp.choice = settings->GetSettingAs<int>("Choice"); // which kind of calculation to invoke
        sphp.max_num = settings->GetSettingAs<int>("Maximum Number of Particles");
        sphp.cloud_num = settings->GetSettingAs<int>("Number of Cloud Particles");
        sphp.max_cloud_num = settings->GetSettingAs<int>("Maximum Number of Cloud Particles");

        //update the OpenCL buffer
        std::vector<SPHParams> vparams(0);
        vparams.push_back(sphp);
        cl_sphp.copyToDevice(vparams);

        settings->updated();
    }


}
