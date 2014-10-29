
#ifndef RTPS_RENDER_H_INCLUDED
#define RTPS_RENDER_H_INCLUDED

#include <map>
#include <GL/glu.h>
#include <GL/gl.h>

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLFunctions>

#include "RTPSettings.h"
#include "../structs.h"
#include "../timer_eb.h"
#include "../opencl/CLL.h"
#include "../opencl/Kernel.h"
#include "../opencl/Buffer.h"
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

    class RTPS_EXPORT Render : protected QOpenGLFunctions
    {
    public:
        Render(GLuint pos_vbo, GLuint vel_vbo, int num, CL *cli, RTPSettings* _settings=0);
        ~Render();

        enum RenderType
        {
            POINTS, SPRITES
        };
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

        void setNum(int nn)
        {
            num = nn;
        }
        void setDepthSmoothing(ShaderType shade)
        {
            smoothing = shade;
        }

        void initBoxBuffer();
        void initShaderProgram();

        void setParticleRadius(float pradius);

        void drawArrays();

        void renderPointsAsSpheres();


        void orthoProjection();
        void perspectiveProjection();
        void fullscreenQuad();

        void renderBox();
        void render_box(float4 min, float4 max); 
        void render_table(float4 min, float4 max); 


        void writeBuffersToDisk();
        void writeFramebufferTextures();
        int writeTexture(GLuint tex, const char* filename);

        enum
        {
            TI_RENDER=0, TI_GLSL
        }; 
        EB::TimerList timers;
        int setupTimers();
        void printTimers();

        virtual void render();
        virtual void setWindowDimensions(GLuint width,GLuint height);

    protected:
        int loadTexture(std::string texture_file, std::string texture_name);
        GLuint compileShaders(const char* vertex_file, const char* fragment_file,
                const char* geometry_file = NULL, GLenum* geom_param=NULL, GLint* geom_value=NULL, int geom_param_len=0);
        std::map<ShaderType,GLuint> glsl_program;    
        std::map<std::string,GLuint> gl_framebuffer_texs;
        std::map<std::string,GLuint> gl_textures;
        virtual void deleteFramebufferTextures();
        virtual void createFramebufferTextures();
        GLuint window_height,window_width;
        GLuint pos_vbo;
        GLuint col_vbo;

        QOpenGLVertexArrayObject m_box_vao;
        QOpenGLBuffer m_box_vbo;
        QOpenGLBuffer m_box_index;

        QOpenGLShaderProgram m_basic_program;

        CL *cli;
        float particle_radius;
        float near_depth;
        float far_depth;
        int num;
        RenderType rtype;
        bool blending;
        bool write_framebuffers;
        ShaderType smoothing;
        std::vector<GLuint> fbos;
        std::vector<GLuint> rbos;
        Buffer<float>   cl_depth;
        Kernel  k_curvature_flow;
        RTPSettings* settings; 
        std::string shader_source_dir;
        int generateCheckerBoardTex(GLubyte* color1,GLubyte* color2,int num_squares, int length);
        int generateCircleTexture(GLubyte r, GLubyte g, GLubyte b, GLubyte alpha, int diameter);
        void convertDepthToRGB(const GLfloat* depth, GLuint size, GLubyte* rgb) const;
        float getParticleRadius()
        {
            return particle_radius;
        }
    };  
}

#endif
