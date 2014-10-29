
#include <render/Render.h>
#include <RTPSettings.h>
#include <RTPS.h>
#include <CLL.h>
#include <system/SPH.h>

namespace rtps
{

    RTPS::RTPS()
    {
        cli = new CL();
        cl_managed = true;
        init();
    }

    RTPS::RTPS(RTPSettings *s)
    {
        cli = new CL();
        cl_managed = true;
        settings = s;
        init();
    }

    RTPS::RTPS(RTPSettings *s, CL* _cli)
    {
        cli = _cli;
        cl_managed = false;
        settings = s;
        init();
    }

    RTPS::~RTPS()
    {
        delete system;
        if(cl_managed)
        {
            delete cli;
        }
    }

    void RTPS::init()
    {
        settings->setMaxOuterParticles(4096*4);
        system = new SPH(this, settings->max_particles, settings->max_outer_particles);
        m_render = new Render(system->getPosVBO(), system->getColVBO(), cli, settings);
    }

    void RTPS::update()
    {
        system->update();
    }

    void RTPS::render()
    {
        m_render->setNum(system->getNum());
        m_render->renderBox();
        m_render->render();
    }

    void RTPS::printTimers()
    {
        system->printTimers();
    }
};

