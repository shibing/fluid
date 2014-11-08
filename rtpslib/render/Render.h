
#ifndef RTPS_RENDER_H_INCLUDED
#define RTPS_RENDER_H_INCLUDED

#include <map>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_3_Core>

#include <structs.h>
#include <rtps_common.h>
#include <timer_eb.h>
#include <opencl/CLL.h>
#include <opencl/Kernel.h>
#include <opencl/Buffer.h>

class QOpenGLTexture;

namespace rtps
{

    class RTPSettings;

    namespace render {

    class RTPS_EXPORT Render : protected QOpenGLFunctions_4_3_Core
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
            POINT = 0,
            SPHERE,
            SURFACE 
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

        void initBoxBuffer();
        void initParticleBuffer();
        void initShaderProgram();
        void initFramebufferObject();

        void setRenderType(RenderType type) { m_render_type = type; }

        void renderBox();
        void renderFluid();
        void renderFluidAsPoint();
        void renderFluidAsSphere();
        void renderFluidAsSurface();

        virtual void setWindowDimensions(GLuint width,GLuint height);
        

        void resetMatrix();
        void moveX(float x);
        void moveZ(float x);

        void rotateX(float x);
        void rotateY(float x);


    private:
        void renderSpriteWithShader(QOpenGLShaderProgram& program);

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

        GLuint m_fbo;
        GLuint m_depth_tex[2];
        GLuint m_thickness_tex[2];
          

        QOpenGLShaderProgram m_basic_program;
        QOpenGLShaderProgram m_particle_program;
        QOpenGLShaderProgram m_sphere_program;
        QOpenGLShaderProgram m_depth_program;
        QOpenGLShaderProgram m_thickness_program;

        CL *m_cli;
        int m_num;

        RTPSettings* m_settings; 
        RenderType m_render_type;

        std::string shader_source_dir;
    };  
    }
}

#endif
