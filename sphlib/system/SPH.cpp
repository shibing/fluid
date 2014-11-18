#include <math.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <iostream>

#include <RTPSettings.h>
#include <system/System.h>
#include <system/SPH.h>
#include <domain/Domain.h>
#include <domain/IV.h>

#include <system/common/Hose.h>

#include<time.h>

    namespace rtps
    {
        using namespace sph;

        SPH::SPH(RTPS *psfr, int n)
        {
            ps = psfr;
            m_push = false;
            m_paused = false;
            settings = ps->settings;
            num = 0;
            max_num = n;
            grid = settings->grid;
            resource_path = settings->GetSettingAs<string>("rtps_path");

            std::vector<SPHParams> vparams(0);
            vparams.push_back(sphp);
            cl_sphp = Buffer<SPHParams>(ps->cli, vparams);

            calculate();
            updateSPHP();

            setupDomain();

            integrator = LEAPFROG;

            prepareSorted();

            ps->cli->addIncludeDir(sph_source_dir);
            ps->cli->addIncludeDir(common_source_dir);

            sph_source_dir = resource_path + "/" + std::string(SPH_CL_SOURCE_DIR);
            common_source_dir = resource_path + "/" + std::string(COMMON_CL_SOURCE_DIR);

            hash = Hash(common_source_dir, ps->cli);
            bitonic = Bitonic<unsigned int>(common_source_dir, ps->cli );
            cellindices = CellIndices(common_source_dir, ps->cli);
            permute = Permute( common_source_dir, ps->cli);

            density = Density(sph_source_dir, ps->cli);
            force = Force(sph_source_dir, ps->cli);
            collision_wall = CollisionWall(sph_source_dir, ps->cli);
            collision_tri = CollisionTriangle(sph_source_dir, ps->cli, 2048); //TODO expose max_triangles as a parameter
            
            if (integrator == LEAPFROG)
            {
                leapfrog = LeapFrog(sph_source_dir, ps->cli);
            }
            else if (integrator == EULER)
            {
                euler = Euler(sph_source_dir, ps->cli);
            }

            string lt_file = settings->GetSettingAs<string>("lt_cl");
    }

    SPH::~SPH()
    {
        Hose* hose;
        int hs = hoses.size();  
        for(int i = 0; i < hs; i++)
        {
            hose = hoses[i];
            delete hose;
        }
    }

    void SPH::update()
    {
        if(m_paused)
            return;
        glFinish();
        if (settings->has_changed()) updateSPHP();

        int sub_intervals =  settings->GetSettingAs<float>("sub_intervals");

        //NOTE: release and acquire opencl buffer
        for (int i=0; i < sub_intervals; i++)
            sprayHoses();

        cl_position_u.acquire();
        cl_color_u.acquire();

        for (int i=0; i < sub_intervals; i++)
        {
            hashAndSort();

            int nc = cellindices.execute(num,
                cl_sort_hashes,
                cl_cell_indices_start,
                cl_cell_indices_end,
                cl_GridParams,
                grid_params.nb_cells,
                clf_debug,
                cli_debug);
            
            permute.execute(num,
                cl_position_u,
                cl_position_s,
                cl_velocity_u,
                cl_velocity_s,
                cl_mass_u,
                cl_mass_s,
                cl_rest_density_u,
                cl_rest_density_s,
                cl_veleval_u,
                cl_veleval_s,
                cl_color_u,
                cl_color_s,
                cl_sort_indices,
                cl_GridParams,
                clf_debug,
                cli_debug);

            //some partilces out of boundary
            if (nc <= num && nc >= 0)
            {
                deleted_pos.resize(num-nc);
                deleted_vel.resize(num-nc);
                cl_position_s.copyToHost(deleted_pos, nc); //damn these will always be out of bounds here!
                cl_velocity_s.copyToHost(deleted_vel, nc);

                settings->SetSetting("Number of Particles", num);
                updateSPHP();
                callPrep(2);
                hashAndSort();
            }

            density.execute(num,
                cl_position_s,
                cl_density_s,
                cl_cell_indices_start,
                cl_cell_indices_end,
                cl_sphp,
                cl_GridParamsScaled, // Might have to fix this. Do not know. 
                clf_debug,
                cli_debug);

            force.execute(num,
                cl_position_s,
                cl_density_s,
                cl_mass_s,
                cl_rest_density_s,
                cl_veleval_s,
                cl_force_s,
                cl_xsph_s,
                cl_cell_indices_start,
                cl_cell_indices_end,
                cl_sphp,
                cl_GridParamsScaled,
                clf_debug,
                cli_debug);
            collision();
            integrate(); 
        }

        cl_position_u.release();
        cl_color_u.release();
    }

	//----------------------------------------------------------------------
    void SPH::hashAndSort()
    {
        hash.execute(num,
                cl_position_u,
                cl_sort_hashes,
                cl_sort_indices,
                cl_GridParams,
                clf_debug,
                cli_debug);
        bitonicSort();
    }

	//----------------------------------------------------------------------
    void SPH::collision()
    {
        int push;
        if(m_push) {
            push = 1;
            m_push = false;
        }
        else
            push = 0;
        collision_wall.execute(num, push,
                cl_position_s,
                cl_velocity_s,
                cl_force_s,
                cl_sphp,
                cl_GridParamsScaled,
                clf_debug,
                cli_debug);

        collision_tri.execute(num,
                settings->dt,
                cl_position_s,
                cl_velocity_s,
                cl_force_s,
                cl_sphp,
                clf_debug,
                cli_debug);
    }

    void SPH::integrate()
    {

        if (integrator == EULER)
        {
            euler.execute(num,
                settings->dt,
                cl_position_u,
                cl_position_s,
                cl_velocity_u,
                cl_velocity_s,
                cl_force_s,
                cl_sort_indices,
                cl_sphp,
                clf_debug,
                cli_debug);
        }
        else if (integrator == LEAPFROG)
        {
             leapfrog.execute(num,
                settings->dt,
                cl_position_u,
                cl_position_s,
                cl_velocity_u,
                cl_velocity_s,
                cl_veleval_u,
                cl_force_s,
                cl_xsph_s,
                cl_sphp,
                clf_debug,
                cli_debug);
        }
		static int count=0;
    }

    void SPH::callPrep(int stage)
    {
        cl_position_u.copyFromBuffer(cl_position_s, 0, 0, num);
        cl_velocity_u.copyFromBuffer(cl_velocity_s, 0, 0, num);
        cl_veleval_u.copyFromBuffer(cl_veleval_s, 0, 0, num);
        cl_color_u.copyFromBuffer(cl_color_s, 0, 0, num);
    }

    void SPH::prepareSorted()
    {
        positions.resize(max_num);
        colors.resize(max_num);
        forces.resize(max_num);
        velocities.resize(max_num);
        veleval.resize(max_num);
        densities.resize(max_num);

        mass.resize(max_num);
        rest_density.resize(max_num);           

        xsphs.resize(max_num);
        std::vector<float4> error_check(max_num);

        std::fill(forces.begin(), forces.end(), float4(0.0f, 0.0f, 1.0f, 0.0f));
        std::fill(velocities.begin(), velocities.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));
        std::fill(veleval.begin(), veleval.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));
        std::fill(densities.begin(), densities.end(), 0.0f);
        std::fill(xsphs.begin(), xsphs.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));
        std::fill(error_check.begin(), error_check.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));

        managed = true;

        m_pos_vbo.create();
        m_pos_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        m_pos_vbo.bind();
        m_pos_vbo.allocate(0, max_num * sizeof(float4));

        m_col_vbo.create();
        m_col_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        m_col_vbo.bind();
        m_col_vbo.allocate(0, max_num * sizeof(float4));

        cl_position_u = Buffer<float4>(ps->cli, m_pos_vbo.bufferId());
        cl_position_s = Buffer<float4>(ps->cli, positions);
        cl_color_u = Buffer<float4>(ps->cli, m_col_vbo.bufferId());
        cl_color_s = Buffer<float4>(ps->cli, colors);

        cl_velocity_u = Buffer<float4>(ps->cli, velocities);
        cl_velocity_s = Buffer<float4>(ps->cli, velocities);
        cl_veleval_u = Buffer<float4>(ps->cli, veleval);
        cl_veleval_s = Buffer<float4>(ps->cli, veleval);
        cl_density_s = Buffer<float>(ps->cli, densities);
        cl_force_s = Buffer<float4>(ps->cli, forces);
        cl_xsph_s = Buffer<float4>(ps->cli, xsphs);
        
        cl_mass_u = Buffer<float>(ps->cli, mass);
        cl_mass_s = Buffer<float>(ps->cli, mass);
        cl_rest_density_u =  Buffer<float>(ps->cli, rest_density);
        cl_rest_density_s =  Buffer<float>(ps->cli, rest_density);

        std::vector<GridParams> gparams(0);
        gparams.push_back(grid_params);
        cl_GridParams = Buffer<GridParams>(ps->cli, gparams);

        std::vector<GridParams> sgparams(0);
        sgparams.push_back(grid_params_scaled);
        cl_GridParamsScaled = Buffer<GridParams>(ps->cli, sgparams);


        std::vector<float4> clfv(max_num);
        std::fill(clfv.begin(), clfv.end(),float4(0.0f, 0.0f, 0.0f, 0.0f));
        std::vector<int4> cliv(max_num);
        std::fill(cliv.begin(), cliv.end(),int4(0.0f, 0.0f, 0.0f, 0.0f));
        clf_debug = Buffer<float4>(ps->cli, clfv);
        cli_debug = Buffer<int4>(ps->cli, cliv);


        std::vector<unsigned int> keys(max_num);
        std::fill(keys.begin(), keys.end(), INT_MAX);
        cl_sort_indices  = Buffer<unsigned int>(ps->cli, keys);
        cl_sort_hashes   = Buffer<unsigned int>(ps->cli, keys);

        std::vector<unsigned int> gcells(grid_params.nb_cells+1);
        int minus = 0xffffffff;
        std::fill(gcells.begin(), gcells.end(), minus);

        cl_cell_indices_start = Buffer<unsigned int>(ps->cli, gcells);
        cl_cell_indices_end   = Buffer<unsigned int>(ps->cli, gcells);

        cl_sort_output_hashes = Buffer<unsigned int>(ps->cli, keys);
        cl_sort_output_indices = Buffer<unsigned int>(ps->cli, keys);
     }

	//----------------------------------------------------------------------
    void SPH::setupDomain()
    {
        grid->calculateCells(sphp.smoothing_distance / sphp.simulation_scale);

        grid_params.grid_min = grid->getMin();
        grid_params.grid_max = grid->getMax();
        grid_params.bnd_min  = grid->getBndMin();
        grid_params.bnd_max  = grid->getBndMax();

        grid_params.grid_res = grid->getRes();
        grid_params.grid_size = grid->getSize();
        grid_params.grid_delta = grid->getDelta();
        grid_params.nb_cells = (int) (grid_params.grid_res.x*grid_params.grid_res.y*grid_params.grid_res.z);

        float ss = sphp.simulation_scale;

        grid_params_scaled.grid_min = grid_params.grid_min * ss;
        grid_params_scaled.grid_max = grid_params.grid_max * ss;
        grid_params_scaled.bnd_min  = grid_params.bnd_min * ss;
        grid_params_scaled.bnd_max  = grid_params.bnd_max * ss;
        grid_params_scaled.grid_res = grid_params.grid_res;
        grid_params_scaled.grid_size = grid_params.grid_size * ss;
        grid_params_scaled.grid_delta = grid_params.grid_delta / ss;
        grid_params_scaled.nb_cells = grid_params.nb_cells;
    }

	//----------------------------------------------------------------------
    int SPH::addBox(FluidType type, int nn, float4 min, float4 max, float4 color)
    {
        float spacing = settings->GetSettingAs<float>("Spacing");
	    vector<float4> rect = addRect(nn, min, max, spacing, 1.0);
        float4 velo(0, 0, 0, 0);
        pushParticles(type, rect, velo, color);
        return rect.size();
    }

    int SPH::addBunny(FluidType type, float4 center)
    {
        vector<float4> particles;
        rtps::addBunny(center, particles);
        pushParticles(type, particles, center);
        return particles.size();
    }

	//----------------------------------------------------------------------
    int SPH::addHose(FluidType type, int total_n, float4 center, float4 velocity, float radius, float4 color)
    {
        float spacing = settings->GetSettingAs<float>("Spacing");
        radius *= spacing;
        Hose *hose = new Hose(ps, total_n, center, velocity, radius, spacing, color);
        hoses.push_back(hose);
        return hoses.size() - 1;
    }

    void SPH::updateHose(FluidType type, int index, float4 center, float4 velocity, float radius, float4 color)
    {
        float spacing = settings->GetSettingAs<float>("Spacing");
        radius *= spacing;
        hoses[index]->update(center, velocity, radius, spacing, color);
    }

    void SPH::refillHose(int index, int refill)
    {
        hoses[index]->refill(refill);
    }

    void SPH::sprayHoses()
    {
        std::vector<float4> parts;
        for (int i = 0; i < hoses.size(); i++)
        {
            parts = hoses[i]->spray();
            if (parts.size() > 0)
                pushParticles(System::WATER, parts, hoses[i]->getVelocity(), hoses[i]->getColor());
        }
    }

    void SPH::testDelete()
    {
        std::vector<float4> poss(40);
        float4 posx(100.,100.,100.,1.);
        std::fill(poss.begin(), poss.end(),posx);
        cl_position_u.acquire();
        cl_position_u.copyToDevice(poss);
        cl_position_u.release();
        ps->cli->queue.finish();
    }
	//----------------------------------------------------------------------
    void SPH::pushParticles(FluidType type, const vector<float4> &pos, const float4& velo, const float4& color)
    {
        int nn = pos.size();
        std::vector<float4> vels(nn);
        std::fill(vels.begin(), vels.end(), velo);
        pushParticles(type, pos, vels, color);

    }
	//----------------------------------------------------------------------
    void SPH::pushParticles(FluidType type, const vector<float4>& pos, const vector<float4>& vels, const float4& color)
    {
        int nn = pos.size();
        if (num + nn > max_num)
        {
			printf("pushParticles: exceeded max nb(%d) of particles allowed\n", max_num);
            return;
        }
        std::vector<float4> cols(nn);
        std::fill(cols.begin(), cols.end(),color);
        
        float mass, rho0;
        if(type == System::WATER) {
            mass = settings->GetSettingAs<float>("Mass");
            rho0 = settings->GetSettingAs<float>("rho0");
        }
        else if(type == System::OIL) {
            mass = settings->GetSettingAs<float>("Mass_Oil");
            rho0 = settings->GetSettingAs<float>("rho0_Oil");
        }

        std::vector<float> mass_v(nn, mass);
        std::vector<float> rho0_v(nn, rho0);

        glFinish();
        cl_position_u.acquire();
        cl_color_u.acquire();

        cl_position_u.copyToDevice(pos, num);
        cl_color_u.copyToDevice(cols, num);
        cl_velocity_u.copyToDevice(vels, num);

        cl_mass_u.copyToDevice(mass_v, num);
        cl_rest_density_u.copyToDevice(rho0_v, num);

        settings->SetSetting("Number of Particles", num + nn);
        updateSPHP();

        cl_position_u.release();
        cl_color_u.release();

        num += nn;  
    }

    void SPH::bitonicSort()
    {
        try
        {
            int dir = 1;        // dir: direction
            int arrayLength = nlpo2(num);
            int batch = 1;

            bitonic.Sort(batch, 
                        arrayLength, 
                        dir,
                        &cl_sort_output_hashes,
                        &cl_sort_output_indices,
                        &cl_sort_hashes,
                        &cl_sort_indices );

        }
        catch (cl::Error er)
        {
            printf("ERROR(bitonic sort): %s(%s)\n", er.what(), oclErrorString(er.err()));
            exit(0);
        }

        ps->cli->queue.finish();
        cl_sort_hashes.copyFromBuffer(cl_sort_output_hashes, 0, 0, num);
        cl_sort_indices.copyFromBuffer(cl_sort_output_indices, 0, 0, num);

        ps->cli->queue.finish();
}
}; //end namespace
