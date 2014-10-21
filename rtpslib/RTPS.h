
#ifndef RTPS_RTPS_H_INCLUDED
#define RTPS_RTPS_H_INCLUDED

#include <vector>

//System API
#include "system/System.h"

//OpenCL API
#include "opencl/CLL.h"

#include "domain/IV.h"

#include "RTPSettings.h"

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

    class RTPS_EXPORT RTPS
    
    {
    public:
        RTPS();
        RTPS(RTPSettings *s);
        RTPS(RTPSettings *s, CL* _cli);

        ~RTPS();

        void Init();

        RTPSettings *settings;
        
        CL *cli;

        System *system;
        System *system_outer; 

        void update();
        void render();

        void printTimers();

    private:
        bool cl_managed;
        
    };
}

#endif
