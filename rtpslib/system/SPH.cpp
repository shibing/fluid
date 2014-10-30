#include <math.h>
#include <sstream>
#include <iomanip>
#include <string>

#include <RTPSettings.h>
#include <system/System.h>
#include <system/SPH.h>
#include "Domain.h"
#include "IV.h"

#include "common/Hose.h"

#include<time.h>

    namespace rtps
    {
        using namespace sph;

        SPH::SPH(RTPS *psfr, int n, int max_nb_in_cloud)
        {
            ps = psfr;
            settings = ps->settings;
            max_num = n;
            num = 0;
            nb_var = 10;


            resource_path = settings->GetSettingAs<string>("rtps_path");

            srand ( time(NULL) );

            grid = settings->grid;

            std::vector<SPHParams> vparams(0);
            vparams.push_back(sphp);
            cl_sphp = Buffer<SPHParams>(ps->cli, vparams);

            calculate();
            updateSPHP();

            spacing = settings->GetSettingAs<float>("Spacing");

            setupDomain();

            integrator = LEAPFROG;

            setupTimers();

            prepareSorted();

            ps->cli->addIncludeDir(sph_source_dir);
            ps->cli->addIncludeDir(common_source_dir);

            sph_source_dir = resource_path + "/" + std::string(SPH_CL_SOURCE_DIR);
            common_source_dir = resource_path + "/" + std::string(COMMON_CL_SOURCE_DIR);

            hash = Hash(common_source_dir, ps->cli, timers["hash_gpu"]);
            bitonic = Bitonic<unsigned int>(common_source_dir, ps->cli );
            radix = Radix<unsigned int>(common_source_dir, ps->cli, max_num, 128);
            cellindices = CellIndices(common_source_dir, ps->cli, timers["ci_gpu"] );
            permute = Permute( common_source_dir, ps->cli, timers["perm_gpu"] );

            density = Density(sph_source_dir, ps->cli, timers["density_gpu"]);
            force = Force(sph_source_dir, ps->cli, timers["force_gpu"]);
            collision_wall = CollisionWall(sph_source_dir, ps->cli, timers["cw_gpu"]);
            collision_tri = CollisionTriangle(sph_source_dir, ps->cli, timers["ct_gpu"], 2048); //TODO expose max_triangles as a parameter
            
            if (integrator == LEAPFROG)
            {
                leapfrog = LeapFrog(sph_source_dir, ps->cli, timers["leapfrog_gpu"]);
            }
            else if (integrator == EULER)
            {
                euler = Euler(sph_source_dir, ps->cli, timers["euler_gpu"]);
            }

            string lt_file = settings->GetSettingAs<string>("lt_cl");
    }

    SPH::~SPH()
    {
        printf("SPH destructor\n");
        if (pos_vbo && managed)
        {
            m_opengl_funcs->glBindBuffer(1, pos_vbo);
            m_opengl_funcs->glDeleteBuffers(1, (GLuint*)&pos_vbo);
            pos_vbo = 0;
        }
        if (col_vbo && managed)
        {
            m_opengl_funcs->glBindBuffer(1, col_vbo);
            m_opengl_funcs->glDeleteBuffers(1, (GLuint*)&col_vbo);
            col_vbo = 0;
        }

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
        timers["update"]->start();
        glFinish();
        if (settings->has_changed()) updateSPHP();

        int sub_intervals =  settings->GetSettingAs<float>("sub_intervals");
        //this should go in the loop but avoiding acquiring and releasing each sub
        //interval for all the other calls.
        //this does end up acquire/release everytime sprayHoses calls pushparticles
        //should just do try/except?
        for (int i=0; i < sub_intervals; i++)
        {
            sprayHoses();
        }

        cl_position_u.acquire();
        cl_color_u.acquire();

        for (int i=0; i < sub_intervals; i++)
        {
            hash_and_sort();

            timers["cellindices"]->start();
            int nc = cellindices.execute(   num,
                cl_sort_hashes,
                cl_sort_indices,
                cl_cell_indices_start,
                cl_cell_indices_end,
                cl_GridParams,
                grid_params.nb_cells,
                clf_debug,
                cli_debug);
            timers["cellindices"]->stop();
            timers["permute"]->start();
            permute.execute(   num,
                cl_position_u,
                cl_position_s,
                cl_velocity_u,
                cl_velocity_s,
                cl_veleval_u,
                cl_veleval_s,
                cl_color_u,
                cl_color_s,
                cl_sort_indices,
                cl_GridParams,
                clf_debug,
                cli_debug);
            timers["permute"]->stop();
 

            if (nc <= num && nc >= 0)
            {
                //check if the number of particles has changed
                //(this happens when particles go out of bounds,
                //  either because of forces or by explicitly placing
                //  them in order to delete)
                //
                //if so we need to copy sorted into unsorted
                //and redo hash_and_sort
                printf("SOME PARTICLES WERE DELETED!\n");
                printf("nc: %d num: %d\n", nc, num);

                deleted_pos.resize(num-nc);
                deleted_vel.resize(num-nc);
                //The deleted particles should be the nc particles after num
                cl_position_s.copyToHost(deleted_pos, nc); //damn these will always be out of bounds here!
                cl_velocity_s.copyToHost(deleted_vel, nc);

                settings->SetSetting("Number of Particles", num);
                updateSPHP();

                call_prep(2);
                hash_and_sort();
                                //we've changed num and copied sorted to unsorted. skip this iteration and do next one
                //this doesn't work because sorted force etc. are having an effect?
                //continue; 
            }


			//-------------------------------------
            //if(num >0) printf("density\n");
            timers["density"]->start();
            density.execute(   num,
                cl_position_s,
                cl_density_s,
                cl_cell_indices_start,
                cl_cell_indices_end,
                cl_sphp,
                cl_GridParamsScaled, // GE: Might have to fix this. Do not know. 
                clf_debug,
                cli_debug);
            timers["density"]->stop();
            
			//-------------------------------------
            //if(num >0) printf("force\n");
            timers["force"]->start();
            force.execute(   num,
                cl_position_s,
                cl_density_s,
                cl_veleval_s,
                cl_force_s,
                cl_xsph_s,
                cl_cell_indices_start,
                cl_cell_indices_end,
                cl_sphp,
                cl_GridParamsScaled,
                clf_debug,
                cli_debug);
            timers["force"]->stop();
            collision();
            integrate(); // includes boundary force
        }

        cl_position_u.release();
        cl_color_u.release();

        timers["update"]->stop();
    }

	//----------------------------------------------------------------------
    void SPH::hash_and_sort()
    {
        timers["hash"]->start();
        hash.execute(   num,
                cl_position_u,
                cl_sort_hashes,
                cl_sort_indices,
                cl_GridParams,
                clf_debug,
                cli_debug);
        timers["hash"]->stop();

        timers["bitonic"]->start();
        bitonic_sort();
        timers["bitonic"]->stop();
    }

	//----------------------------------------------------------------------
    void SPH::collision()
    {
        //when implemented other collision routines can be chosen here
        timers["collision_wall"]->start();
        collision_wall.execute(num,
                cl_position_s,
                cl_velocity_s,
                cl_force_s,
                cl_sphp,
                cl_GridParamsScaled,
                clf_debug,
                cli_debug);

        timers["collision_wall"]->stop();

        timers["collision_tri"]->start();
        collision_tri.execute(num,
                settings->dt,
                cl_position_s,
                cl_velocity_s,
                cl_force_s,
                cl_sphp,
                clf_debug,
                cli_debug);
        timers["collision_tri"]->stop();
    }
	//----------------------------------------------------------------------

    void SPH::integrate()
    {
        timers["integrate"]->start();

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
                cl_sort_indices,
                cl_sphp,
                clf_debug,
                cli_debug);
        }
		static int count=0;
    	timers["integrate"]->stop();
    }

    void SPH::call_prep(int stage)
    {
		// copy from sorted to unsorted arrays at the beginning of each 
		// iteration
		// copy from cl_position_s to cl_position_u
		// Only called if number of fluid particles changes from one iteration
		// to the other

            cl_position_u.copyFromBuffer(cl_position_s, 0, 0, num);
            cl_velocity_u.copyFromBuffer(cl_velocity_s, 0, 0, num);
            cl_veleval_u.copyFromBuffer(cl_veleval_s, 0, 0, num);
            cl_color_u.copyFromBuffer(cl_color_s, 0, 0, num);
    }

	//----------------------------------------------------------------------
    int SPH::setupTimers()
    {
        int print_freq = 1000; //one second
        int time_offset = 5;
        timers["update"] = new EB::Timer("Update loop", time_offset);
        timers["hash"] = new EB::Timer("Hash function", time_offset);
        timers["hash_gpu"] = new EB::Timer("Hash GPU kernel execution", time_offset);
        timers["cellindices"] = new EB::Timer("CellIndices function", time_offset);
        timers["ci_gpu"] = new EB::Timer("CellIndices GPU kernel execution", time_offset);
        timers["permute"] = new EB::Timer("Permute function", time_offset);
        timers["cloud_permute"] = new EB::Timer("CloudPermute function", time_offset);
        timers["perm_gpu"] = new EB::Timer("Permute GPU kernel execution", time_offset);
        timers["ds_gpu"] = new EB::Timer("DataStructures GPU kernel execution", time_offset);
        timers["bitonic"] = new EB::Timer("Bitonic Sort function", time_offset);
        timers["density"] = new EB::Timer("Density function", time_offset);
        timers["density_gpu"] = new EB::Timer("Density GPU kernel execution", time_offset);
        timers["force"] = new EB::Timer("Force function", time_offset);
        timers["force_gpu"] = new EB::Timer("Force GPU kernel execution", time_offset);
        timers["collision_wall"] = new EB::Timer("Collision wall function", time_offset);
        timers["cw_gpu"] = new EB::Timer("Collision Wall GPU kernel execution", time_offset);
        timers["collision_tri"] = new EB::Timer("Collision triangles function", time_offset);
        timers["ct_gpu"] = new EB::Timer("Collision Triangle GPU kernel execution", time_offset);
        timers["collision_cloud"] = new EB::Timer("Collision cloud function", time_offset);
        timers["integrate"] = new EB::Timer("Integration function", time_offset);
        timers["leapfrog_gpu"] = new EB::Timer("LeapFrog Integration GPU kernel execution", time_offset);
        timers["euler_gpu"] = new EB::Timer("Euler Integration GPU kernel execution", time_offset);
		return 0;
    }

	//----------------------------------------------------------------------
    void SPH::printTimers()
    {
        printf("Number of Particles: %d\n", num);
        timers.printAll();
        std::ostringstream oss; 
        oss << "sph_timer_log_" << std::setw( 7 ) << std::setfill( '0' ) <<  num; 
        timers.writeToFile(oss.str()); 
    }

	//----------------------------------------------------------------------
    void SPH::prepareSorted()
    {

        positions.resize(max_num);
        colors.resize(max_num);
        forces.resize(max_num);
        velocities.resize(max_num);
        veleval.resize(max_num);
        densities.resize(max_num);
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

        //vbo buffers
        cl_position_u = Buffer<float4>(ps->cli, m_pos_vbo.bufferId());
        cl_position_s = Buffer<float4>(ps->cli, positions);
        cl_color_u = Buffer<float4>(ps->cli, m_col_vbo.bufferId());
        cl_color_s = Buffer<float4>(ps->cli, colors);

        //pure opencl buffers: these are deprecated
        cl_velocity_u = Buffer<float4>(ps->cli, velocities);
        cl_velocity_s = Buffer<float4>(ps->cli, velocities);
        cl_veleval_u = Buffer<float4>(ps->cli, veleval);
        cl_veleval_s = Buffer<float4>(ps->cli, veleval);
        cl_density_s = Buffer<float>(ps->cli, densities);
        cl_force_s = Buffer<float4>(ps->cli, forces);
        cl_xsph_s = Buffer<float4>(ps->cli, xsphs);

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
        std::fill(gcells.begin(), gcells.end(), 666);

        cl_cell_indices_start = Buffer<unsigned int>(ps->cli, gcells);
        cl_cell_indices_end   = Buffer<unsigned int>(ps->cli, gcells);

        cl_sort_output_hashes = Buffer<unsigned int>(ps->cli, keys);
        cl_sort_output_indices = Buffer<unsigned int>(ps->cli, keys);

		printf("keys.size= %d\n", keys.size()); // 
		printf("gcells.size= %d\n", gcells.size()); // 1729
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
    int SPH::addBox(int nn, float4 min, float4 max, bool scaled, float4 color)
    {
        float scale = 1.0f;
	    vector<float4> rect = addRect(nn, min, max, spacing, scale);
        float4 velo(0, 0, 0, 0);
        pushParticles(rect, velo, color);
        return rect.size();
    }

    void SPH::addBall(int nn, float4 center, float radius, bool scaled)
    {
        float scale = 1.0f;
        if (scaled) scale = sphp.simulation_scale;
        vector<float4> sphere = addSphere(nn, center, radius, spacing, scale);
        float4 velo(0, 0, 0, 0);
        pushParticles(sphere,velo);
    }

	//----------------------------------------------------------------------
    int SPH::addHose(int total_n, float4 center, float4 velocity, float radius, float4 color)
    {
        radius *= spacing;
        Hose *hose = new Hose(ps, total_n, center, velocity, radius, spacing, color);
        hoses.push_back(hose);
        return hoses.size() - 1;
    }
    void SPH::updateHose(int index, float4 center, float4 velocity, float radius, float4 color)
    {
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
                pushParticles(parts, hoses[i]->getVelocity(), hoses[i]->getColor());
        }
    }

    void SPH::testDelete()
    {

        //cut = 1;
        std::vector<float4> poss(40);
        float4 posx(100.,100.,100.,1.);
        std::fill(poss.begin(), poss.end(),posx);
        //cl_vars_unsorted.copyToDevice(poss, max_num + 2);
        cl_position_u.acquire();
        cl_position_u.copyToDevice(poss);
        cl_position_u.release();
        ps->cli->queue.finish();
    }
	//----------------------------------------------------------------------
    void SPH::pushParticles(vector<float4> pos, float4 velo, float4 color)
    {
        int nn = pos.size();
        std::vector<float4> vels(nn);
        std::fill(vels.begin(), vels.end(), velo);
        pushParticles(pos, vels, color);

    }
	//----------------------------------------------------------------------
    void SPH::pushParticles(vector<float4> pos, vector<float4> vels, float4 color)
    {

        int nn = pos.size();
        if (num + nn > max_num)
        {
			printf("pushParticles: exceeded max nb(%d) of particles allowed\n", max_num);
            return;
        }
        std::vector<float4> cols(nn);
        std::fill(cols.begin(), cols.end(),color);

        glFinish();
        cl_position_u.acquire();
        cl_color_u.acquire();

        cl_position_u.copyToDevice(pos, num);
        cl_color_u.copyToDevice(cols, num);
        cl_velocity_u.copyToDevice(vels, num);

        settings->SetSetting("Number of Particles", num + nn);
        updateSPHP();

        cl_position_u.release();
        cl_color_u.release();

        num += nn;  
    }

    void SPH::radix_sort()
    {
    try 
        {   
            int snum = nlpo2(num);
            if(snum < 1024)
            {   
                snum = 1024;
            }   
            radix.sort(snum, &cl_sort_hashes, &cl_sort_indices);
        }   
        catch (cl::Error er) 
        {   
            printf("ERROR(radix sort): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }   

    }
	//----------------------------------------------------------------------
    void SPH::bitonic_sort()
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
