
#ifndef RTPS_RENDER_H_INCLUDED
#define RTPS_RENDER_H_INCLUDED

#include <map>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>


#include <structs.h>
#include <rtps_common.h>
#include <timer_eb.h>
#include <opencl/CLL.h>
#include <opencl/Kernel.h>
#include <opencl/Buffer.h>

class QOpenGLFunctions_4_3_Core;
namespace rtps
{

    class RTPSettings;

    namespace render {

    class RTPS_EXPORT Render 
    {
    public:
        enum ShaderType
        {
            NO_SHADER,SPHERE_SHADER,
            DEPTH_SHADER,
            GAUSSIAN_X_SHADER,
            GAUSSIAN_Y_SHADER,
            BILATERAL_GAUSSIAN_SHADER,
            NORMAL_SHADER,
            CURVATURE_FLOW_SHADER,
            MIKEP_SHADER,
            COPY_TO_FB
        };

        enum RenderType
        {
            POINT,
            SPHERE,

        };
    public:
        Render(QOpenGLBuffer pos_vbo, QOpenGLBuffer col_vbo, CL *cli, RTPSettings* settings=0, RenderType type = POINT);
        ~Render();


        void setNum(int nn)
        {
            m_num = nn;
        }
        enum
        {
            TI_RENDER=0, TI_GLSL
        }; 


        void setOpenGLFunctions(QOpenGLFunctions_4_3_Core*funcs) { m_opengl_funcs = funcs; }
        QOpenGLFunctions_4_3_Core * getOpenGLFunctions() const { return m_opengl_funcs; }
        
        void initBoxBuffer();
        void initParticleBuffer();
        void initShaderProgram();

        void setRenderType(RenderType type) { m_render_type = type; }

        void renderBox();
        void renderFluid();
        void renderFluidAsPoint();
        void renderFluidAsSphere();

        virtual void setWindowDimensions(GLuint width,GLuint height);
        

        void resetMatrix();
        void moveX(float x);
        void moveZ(float x);

        void rotateX(float x);
        void rotateY(float x);

    protected:
        GLuint window_height,window_width;

        QMatrix4x4 m_perspective_mat;
        QMatrix4x4 m_modelview_mat;
        QMatrix4x4 m_rotate_mat;

        QOpenGLVertexArrayObject m_particle_vao;
        QOpenGLBuffer m_pos_vbo;
        QOpenGLBuffer m_col_vbo;

        QOpenGLVertexArrayObject m_box_vao;
        QOpenGLBuffer m_box_vbo;
        QOpenGLBuffer m_box_index;

        QOpenGLShaderProgram m_basic_program;
        QOpenGLShaderProgram m_particle_program;
        QOpenGLShaderProgram m_sphere_program;

        QOpenGLFunctions_4_3_Core *m_opengl_funcs;

        CL *m_cli;
        int m_num;

        RTPSettings* m_settings; 
        RenderType m_render_type;

        std::string shader_source_dir;
    };  
    }
}

#endif
