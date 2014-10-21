
#ifndef RTPS_SYSTEM_H_INCLUDED
#define RTPS_SYSTEM_H_INCLUDED

#include "../domain/Domain.h"
#include "ForceField.h"
#include "../render/Render.h"
#include "../render/SpriteRender.h"
#include "../render/SSFRender.h"
#include "../render/Sphere3DRender.h"

#ifdef WIN32
    #if defined(rtps_EXPORTS)
        #define RTPS_EXPORT __declspec(dllexport)
    #else
        #define RTPS_EXPORT __declspec(dllimport)
	#endif 
#else
    #define RTPS_EXPORT
#endif

#include<stdio.h>
namespace rtps
{

    class RTPS_EXPORT System
    {
    public:
        virtual void update() = 0;

        virtual ~System()
        {
            delete renderer;
        }

        virtual Domain* getGrid()
        {
            return grid;
        }
        virtual int getNum()
        {
            return num;
        }
        virtual void setNum(int nn)
        {
            num = nn;
        };
        virtual GLuint getPosVBO()
        {
            return pos_vbo;
        }
        virtual GLuint getColVBO()
        {
            return col_vbo;
        }

        virtual void render()
        {
            renderer->render();
        }

        virtual int addBox(int nn, float4 min, float4 max, bool scaled, float4 color=float4(1., 0., 0., 1.))
        {
            return 0;
        };

        virtual void addBall(int nn, float4 center, float radius, bool scaled, float4 color=float4(1., 0., 0., 1.))
        {
        };
        virtual int addHose(int total_n, float4 center, float4 velocity, float radius, float4 color=float4(1., 0., 0., 1.))
        {
            return 0;
        };
        virtual void updateHose(int index, float4 center, float4 velocity, float radius, float4 color=float4(1., 0., 0., 1.))
        {
        };
        virtual void refillHose(int index, int refill)
        {
        };
 
        virtual void sprayHoses()
        {
        };
        virtual void testDelete()
        {
        };


        virtual void loadTriangles(std::vector<Triangle> &triangles)
        {

        };

        virtual void addForceField(ForceField ff)
        {
        };


        virtual void printTimers()
        {
            renderer->printTimers();
        };

        virtual Render* getRenderer()
        {
            return renderer;
        }

    protected:
        int num;  
        int max_num;

        GLuint pos_vbo;
        GLuint col_vbo;
        bool managed;

        Domain* grid;

        Render* renderer;

        std::string resource_path;
        std::string common_source_dir;

        virtual void setRenderer()
        {
        }

    };

}

#endif
