
#include <GL/glew.h>
#include "RTPS.h"
#include "system/SPH.h"


namespace rtps
{

    RTPS::RTPS()
    {
        cli = new CL();
        cl_managed = true;
        Init();
    }

    RTPS::RTPS(RTPSettings *s)
    {
        cli = new CL();
        cl_managed = true;
        settings = s;
        Init();
    }

    RTPS::RTPS(RTPSettings *s, CL* _cli)
    {
        cli = _cli;
        cl_managed = false;
        settings = s;
        Init();
    }

    RTPS::~RTPS()
    {
        delete system;
        if(cl_managed)
        {
            delete cli;
        }
    }

    void RTPS::Init()
    {
        glewInit();
        settings->setMaxOuterParticles(4096*4);
        system = new SPH(this, settings->max_particles, settings->max_outer_particles);
    }

    void RTPS::update()
    {
        system->update();
    }

    void RTPS::render()
    {
        system->render();
    }

    void RTPS::printTimers()
    {
            system->printTimers();
    }
};

