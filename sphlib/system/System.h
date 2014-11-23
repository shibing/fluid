
#ifndef RTPS_SYSTEM_H_INCLUDED
#define RTPS_SYSTEM_H_INCLUDED

#include <render/Render.h>

#include <domain/Domain.h>

#include <QOpenGLBuffer>
#include <QOpenGLFunctions_4_3_Core>
#include <rtps_common.h>

namespace rtps
{
    class Domain;

    class RTPS_EXPORT System
    {
    public:
        System(): m_pos_vbo(QOpenGLBuffer::VertexBuffer), m_col_vbo(QOpenGLBuffer::VertexBuffer) { }

        virtual void update() = 0;

        virtual ~System()
        {
        }

        virtual Domain* getGrid()
        {
            return grid;
        }

        virtual int getNum()
        {
            return num;
        }

        void setOpenGLFunctions(QOpenGLFunctions_4_3_Core *funcs) { m_opengl_funcs = funcs; }

        virtual void setNum(int nn)
        {
            num = nn;
        };

        virtual void setPush() = 0;
        virtual void setPaused() = 0;
        virtual QOpenGLBuffer getPosVBO() { return m_pos_vbo; }

        virtual QOpenGLBuffer getColVBO() { return m_col_vbo; }

        virtual int addBox(int nn, float4 min, float4 max, bool scaled, float4 color=float4(1., 0., 0., 1.)) { return 0; }

        virtual void addBall(int nn, float4 center, float radius, bool scaled, float4 color=float4(1., 0., 0., 1.)) { } 

        virtual int addHose(int total_n, float4 center, float4 velocity, float radius, float4 color=float4(1., 0., 0., 1.)) { return 0; }
        virtual int addBunny(const float4& center, const float4& color) = 0;

        virtual void updateHose(int index, float4 center, float4 velocity, float radius, float4 color=float4(1., 0., 0., 1.)) { }
        virtual void refillHose(int index, int refill) { }
 
        virtual void sprayHoses() {}
        virtual void testDelete() {}
        virtual void loadTriangles(std::vector<Triangle> &triangles) {}

    protected:
        int num;  
        int max_num;
        bool managed;
        Domain* grid;
        std::string resource_path;
        std::string common_source_dir;

        QOpenGLBuffer m_pos_vbo;
        QOpenGLBuffer m_col_vbo;
        QOpenGLFunctions_4_3_Core *m_opengl_funcs;


    };

}

#endif
