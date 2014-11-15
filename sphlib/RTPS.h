
#ifndef RTPS_RTPS_H_INCLUDED
#define RTPS_RTPS_H_INCLUDED

#include <vector>

#include <domain/IV.h>

#include <structs.h>

#include <util.h>

#include <rtps_common.h>


namespace rtps
{
    class System;
    class CL;
    class RTPSettings;
    namespace render {
        class Render;
    }

    class RTPS_EXPORT RTPS
    
    {
    public:
        RTPS();
        RTPS(RTPSettings *s);
        RTPS(RTPSettings *s, CL* _cli);

        ~RTPS();

        void init();

        render::Render * getRender() const { return m_render; }
        void setRender(render::Render *render) { m_render = render; }

        System * getSystem() const { return system; }
        void setSystem(System *system) { this->system = system; }

        RTPSettings *settings;
        
        CL *cli;

        System *system;

        void update();
        void render();

    private:
        render::Render *m_render;
        bool cl_managed;
        
    };
}

#endif
