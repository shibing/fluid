
#include <RTPSettings.h>
namespace rtps
{
    unsigned int nlpo2(register unsigned int x)
    {
        x--;
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        return(x+1);
    }

    RTPSettings::RTPSettings()
    {
        changed = false;
        system = SPH;
        max_particles = 2048;
        max_outer_particles = 0;
        dt = .001f;
        grid = new Domain(float4(-5,-.3f,0,0), float4(2, 2, 12, 0));
    }

    RTPSettings::RTPSettings(SysType system, int max_particles, float dt)
    {
        changed = false;
        this->system = system;
        this->max_particles = max_particles;
        this->max_outer_particles = 0;
        this->dt = dt;
        grid = new Domain(float4(-5,-.3f,0,0), float4(2, 2, 12, 0));
    }

    RTPSettings::RTPSettings(SysType system, int max_particles, float dt, Domain* grid)
    {
        changed = false;
        this->system = system;
        this->max_particles = nlpo2(max_particles);
        this->max_outer_particles = 0;
        this->dt = dt;
        this->grid = grid;
    }


    RTPSettings::~RTPSettings()
    {
    }

    void RTPSettings::printSettings()
    {
        typedef std::map <std::string, std::string> MapType;

        MapType::const_iterator end = settings.end();
        for(MapType::const_iterator it = settings.begin(); it != end; ++it)
        {
            printf("%s: %s\n", it->first.c_str(), it->second.c_str());
        }
    }

}
