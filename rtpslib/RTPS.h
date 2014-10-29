
#ifndef RTPS_RTPS_H_INCLUDED
#define RTPS_RTPS_H_INCLUDED

#include <vector>

#include "domain/IV.h"

#include "structs.h"

#include "util.h"

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
    class System;
    class CL;
    class RTPSettings;
    class Render;

    class RTPS_EXPORT RTPS
    
    {
    public:
        RTPS();
        RTPS(RTPSettings *s);
        RTPS(RTPSettings *s, CL* _cli);

        ~RTPS();

        void init();

        Render * getRender() const { return m_render; }
        void setRender(Render *render) { m_render = render; }

        System * getSystem() const { return system; }
        void setSystem(System *system) { this->system = system; }



        RTPSettings *settings;
        
        CL *cli;

        System *system;
        System *system_outer; 

        void update();
        void render();

        void printTimers();

    private:
        Render *m_render;
        bool cl_managed;
        
    };
}

#endif
