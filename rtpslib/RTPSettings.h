#ifndef RTPS_RTPSETTINGS_H_INCLUDED
#define RTPS_RTPSETTINGS_H_INCLUDED

#include <stdlib.h>
#include <string>
#include <map>
#include <iostream>
#include <stdio.h>
#include <sstream>


#include "domain/Domain.h"
#ifdef WIN32
    #if defined(rtps_EXPORTS)
        #define RTPS_EXPORT __declspec(dllexport)
    #else
        #define RTPS_EXPORT __declspec(dllimport)
	#endif 
#else
    #define RTPS_EXPORT
#endif

namespace rtps
{
    //next largest power of 2. hack required for BitonicSort
    unsigned int nlpo2(register unsigned int x);

    class RTPS_EXPORT RTPSettings 
    {
    public:
        enum SysType
        {
            Simple, SPH, FLOCK, OUTER
        };
        SysType system;

        enum RenderType
        {
            RENDER = 0, SPRITE_RENDER, SCREEN_SPACE_RENDER, SPHERE3D_RENDER
        };


        RTPSettings();
        RTPSettings(SysType system, int max_particles, float dt);
        RTPSettings(SysType system, int max_particles, float dt, Domain *grid);

        //collision
        RTPSettings(SysType system, int max_particles, float dt, Domain *grid, bool tri_collision);

        //flock
        RTPSettings(SysType system, int max_particles, float dt, Domain* grid, float maxspeed, float mindist, float searchradius, float color[], float w_sep, float w_align, float w_coh);

		// (GE) TEMPORARY WHILE WAITING FOR REFACTORING
		void setMaxOuterParticles(int max_outer_particles) {
			this->max_outer_particles = nlpo2(max_outer_particles);
		}
		int getMaxOuterParticles() {
			return this->max_outer_particles;
		}

        ~RTPSettings();

        int max_particles;
        int max_outer_particles; 
        Domain *grid; 
        //time step per iteration
        float dt;
        bool tri_collision;

        // FLOCK: target of goal rule
        float4 target;

        // FLOCK: 2D simulation
        bool two_dimensional;

        bool has_changed() { return changed; };
        void updated() { changed = false; }; //one system using one setting 

        void printSettings();

	    template <typename RT>
        RT GetSettingAs(std::string key, std::string defaultval = "0") 
        {
            if (settings.find(key) == settings.end()) 
            {
                RT ret = ss_typecast<RT>(defaultval);
                return ret;
            }
            return ss_typecast<RT>(settings[key]);
        }

        template <typename RT>
        void SetSetting(std::string key, RT value) {
            std::ostringstream oss; 
            oss << value; 
            settings[key] = oss.str(); 
            changed = true;
        }
    
        bool Exists(std::string key) { if(settings.find(key) == settings.end()) { return false; } else { return true; } }

    private:
        std::map<std::string, std::string> settings;
        bool changed;
        
        float render_radius_scale;
        float render_blur_scale;
        RenderType render_type;
        bool use_glsl;
        bool use_alpha_blending;

        template<typename RT, typename _CharT, typename _Traits , typename _Alloc >
        RT ss_typecast( const std::basic_string< _CharT, _Traits, _Alloc >& the_string )
        {
            std::basic_istringstream< _CharT, _Traits, _Alloc > temp_ss(the_string);
            RT num;
            temp_ss >> num;
            return num;
        }

    public:
        Domain* getDomain()
        {
            return grid;
        }
        void setDomain(Domain *domain)
        {
            grid = domain;
        }

        float getRadiusScale()
        {
            return render_radius_scale;
        }
        void setRadiusScale(float scale)
        {
            render_radius_scale = scale;
        }

        float getBlurScale()
        {
            return render_blur_scale;
        }
        void setBlurScale(float scale)
        {
            render_blur_scale = scale;
        }

        int getRenderType()
        {
            return render_type;
        }
        void setRenderType(RenderType type)
        {
            render_type = type;
        }

        int getUseAlphaBlending()
        {
            return use_alpha_blending;
        }
        void setUseAlphaBlending(bool use_alpha)
        {
            use_alpha_blending = use_alpha;
        }

        int getUseGLSL()
        {
            return use_glsl;
        }
        void setUseGLSL(bool use_glsl)
        {
            this->use_glsl = use_glsl;
        }

        float4 getTarget()
        {
            return target;
        }
        void setTarget(float4 t)
        {
            target = t;
            target.print("target");
        }

        int getDimension()
        {
            return two_dimensional;
        }
        void setDimension(bool dim)
        {
            two_dimensional = dim;
        }
    };

}

#endif
