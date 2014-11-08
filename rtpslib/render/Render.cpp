
#include <QOpenGLFunctions_4_3_Core>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <QOpenGLTexture>

#include <RTPSettings.h>

#include <render/Render.h>
#include <render/quad.h>

using namespace std;

namespace rtps
{
    namespace render {

    Render::Render(QOpenGLBuffer pos_vbo, QOpenGLBuffer col_vbo, CL* cli,  RTPSettings* settings, RenderType type) :
        m_num(0),
        m_render_type(type),
        m_cli(cli),
        m_settings(settings),
        m_pos_vbo(pos_vbo),
        m_col_vbo(col_vbo),
        m_box_vbo(QOpenGLBuffer::VertexBuffer),
        m_box_index(QOpenGLBuffer::IndexBuffer)
    {
        shader_source_dir = m_settings->GetSettingAs<string>("rtps_path");
        shader_source_dir += "/shaders";
        initializeOpenGLFunctions();

        initShaderProgram();
        initBoxBuffer();
        initParticleBuffer();
        initFramebufferObject();
        resetMatrix();
    }

    Render::~Render()
    {
        
    }

    void Render::initBoxBuffer()
    {
        float4 min = m_settings->grid->getBndMin();
        float4 max = m_settings->grid->getBndMax();

        static float position_buf[] = { 
            min.x, min.y, min.z,
            min.x, min.y, max.z,
            max.x, min.y, max.z,
            max.x, min.y, min.z,

            min.x, max.y, min.z,
            min.x, max.y, max.z,
            max.x, max.y, max.z,
            max.x, max.y, min.z,
        };

        static unsigned int index_buf[] = {
            0, 1,
            1, 2,
            2, 3,
            3, 0,

            0, 4,
            1, 5,
            2, 6,
            3, 7,

            4, 5,
            5, 6,
            6, 7,
            7, 4
        };

        m_box_vao.create();
        m_box_vao.bind();

        m_box_vbo.create();
        m_box_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
        m_box_vbo.bind();
        m_box_vbo.allocate(&position_buf[0], sizeof(position_buf));
        m_basic_program.enableAttributeArray(0);
        m_basic_program.setAttributeBuffer(0, GL_FLOAT, 0, 3);
        
        m_box_index.create();
        m_box_index.setUsagePattern(QOpenGLBuffer::StaticDraw);
        m_box_index.bind();
        m_box_index.allocate(&index_buf[0], sizeof(index_buf));

        m_box_vao.release();
    }

    void Render::initParticleBuffer()
    {
        m_particle_vao.create();
        m_particle_vao.bind();
        m_pos_vbo.bind();
        /* m_particle_program.enableAttributeArray(0); */
        /* m_particle_program.setAttributeBuffer(0, GL_FLOAT, 0, 4); */
        m_sphere_program.enableAttributeArray(0);
        m_sphere_program.setAttributeBuffer(0, GL_FLOAT, 0, 4);
        m_particle_vao.release();
    }

    void Render::initShaderProgram()
    {
        assert(m_basic_program.addShaderFromSourceFile(QOpenGLShader::Vertex, (shader_source_dir + "/basic.vert").c_str()));
        assert(m_basic_program.addShaderFromSourceFile(QOpenGLShader::Fragment, (shader_source_dir + "/basic.frag").c_str()));
        assert(m_basic_program.link());

        assert(m_particle_program.addShaderFromSourceFile(QOpenGLShader::Vertex, (shader_source_dir + "/point.vert").c_str()));
        assert( m_particle_program.addShaderFromSourceFile(QOpenGLShader::Fragment, (shader_source_dir + "/point.frag").c_str()));
        assert(m_particle_program.link());

        assert(m_sphere_program.addShaderFromSourceFile(QOpenGLShader::Vertex, (shader_source_dir + "/sphere.vs").c_str()));
        assert(m_sphere_program.addShaderFromSourceFile(QOpenGLShader::Fragment, (shader_source_dir + "/sphere.fs").c_str()));
        assert(m_sphere_program.link());

        assert(m_depth_program.addShaderFromSourceFile(QOpenGLShader::Vertex, (shader_source_dir + "/depth.vs").c_str()));
        assert(m_depth_program.addShaderFromSourceFile(QOpenGLShader::Fragment, (shader_source_dir + "/depth.fs").c_str()));
        assert(m_depth_program.link());

        assert(m_thickness_program.addShaderFromSourceFile(QOpenGLShader::Vertex, (shader_source_dir + "/thickness.vs").c_str()));
        assert(m_thickness_program.addShaderFromSourceFile(QOpenGLShader::Fragment, (shader_source_dir + "/thickness.fs").c_str()));
        assert(m_thickness_program.link());

    }

    void Render::initFramebufferObject()
    {
        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");
        glGenTextures(2, &m_depth_tex[0]);
        for(int i = 0; i < 2; ++i) {
            glBindTexture(GL_TEXTURE_2D, m_depth_tex[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glGenTextures(2, &m_thickness_tex[0]);
        for(int i = 0; i < 2; ++i) {
            glBindTexture(GL_TEXTURE_2D, m_thickness_tex[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        GLuint rb;
        glGenRenderbuffers(1, &rb);
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);

        GLenum re;
        if((re = glCheckFramebufferStatus(GL_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE)
            std::cout << "frame bufer set done" << std::endl;
        else {
            std::cout << "frame buffer incomplete" << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Render::renderFluid()
    {
        switch (m_render_type) {
            case Render::POINT:
                renderFluidAsPoint();
                break;
            case Render::SPHERE:
                renderFluidAsSphere();
                break;
            case Render::SURFACE:
                renderFluidAsSurface();
                break;
        }
    }

    void Render::renderSpriteWithShader(QOpenGLShaderProgram& program)
    {
        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");
        m_perspective_mat.setToIdentity();
        m_perspective_mat.perspective(60.0f, width/(height * 1.0f), 0.1, 1000.0f);
        
        glEnable(GL_PROGRAM_POINT_SIZE);

        m_particle_vao.bind();
        glDrawArrays(GL_POINTS, 0, m_num);
        glFinish(); 
        glDisable(GL_PROGRAM_POINT_SIZE);

        m_particle_vao.release();
        
    }

    void Render::renderBox()
    {
        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");
        m_perspective_mat.setToIdentity();
        m_perspective_mat.perspective(60.0f, width/(height * 1.0f), 0.1, 1000.0f);

        glEnable(GL_DEPTH_TEST);
        m_basic_program.bind();
        GLuint uniform_matrix = m_basic_program.uniformLocation("matrix");
        m_basic_program.setUniformValue(uniform_matrix, m_perspective_mat * m_modelview_mat * m_rotate_mat);
        m_box_vao.bind();
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        m_basic_program.release();
    }

    void Render::renderFluidAsPoint()
    {
        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");
        m_perspective_mat.setToIdentity();
        m_perspective_mat.perspective(60.0f, width/(height * 1.0f), 0.1, 1000.0f);
        
        glPointSize(5.0f);
        glEnable(GL_DEPTH_TEST);

        m_particle_program.bind();
        m_particle_vao.bind();

        GLuint uniform_matrix = m_basic_program.uniformLocation("matrix");
        m_particle_program.setUniformValue(uniform_matrix, m_perspective_mat * m_modelview_mat * m_rotate_mat);
        glDrawArrays(GL_POINTS, 0, m_num);
        glFinish(); 

        m_particle_vao.release();
        m_particle_program.release();
    }

    void Render::renderFluidAsSphere()
    {
        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");

        m_sphere_program.bind();

        m_sphere_program.setUniformValue("modelview_mat", m_modelview_mat * m_rotate_mat);
        m_sphere_program.setUniformValue("projection_mat" , m_perspective_mat);
        m_sphere_program.setUniformValue("sphere_radius", m_settings->GetSettingAs<float>("Spacing") / 2.0f);
        m_sphere_program.setUniformValue("near", 0.1f);
        m_sphere_program.setUniformValue("far", 1000.0f);
        m_sphere_program.setUniformValue("width", width);
        m_sphere_program.setUniformValue("height", height);

        glViewport(0, 0, width, height);
        glEnable(GL_DEPTH_TEST);
        renderSpriteWithShader(m_sphere_program);
        glDisable(GL_DEPTH_TEST);

        m_sphere_program.bind();
    }

    void Render::renderFluidAsSurface()
    {
        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");

        m_perspective_mat.setToIdentity();
        m_perspective_mat.perspective(60.0f, width/(height * 1.0f), 1.0, 100.0f);

        //draw depth
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_depth_tex[0], 0);

        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, width, height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_depth_program.bind();
        m_depth_program.setUniformValue("modelview_mat", m_modelview_mat * m_rotate_mat);
        m_depth_program.setUniformValue("projection_mat" , m_perspective_mat);
        m_depth_program.setUniformValue("sphere_radius", m_settings->GetSettingAs<float>("Spacing") / 2.0f);
        m_depth_program.setUniformValue("near", 0.1f);
        m_depth_program.setUniformValue("far", 1000.0f);
        m_depth_program.setUniformValue("width", width);
        m_depth_program.setUniformValue("height", height);

        renderSpriteWithShader(m_depth_program);

        glDisable(GL_DEPTH_TEST);

        m_depth_program.release();

        //draw thickness

        m_thickness_program.bind();

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_thickness_tex[0], 0);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glViewport(0, 0, width, height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        m_thickness_program.bind();
        m_thickness_program.setUniformValue("modelview_mat", m_modelview_mat * m_rotate_mat);
        m_thickness_program.setUniformValue("projection_mat" , m_perspective_mat);
        m_thickness_program.setUniformValue("sphere_radius", m_settings->GetSettingAs<float>("Spacing") / 2.0f);
        m_thickness_program.setUniformValue("near", 0.1f);
        m_thickness_program.setUniformValue("far", 1000.0f);
        m_thickness_program.setUniformValue("width", width);
        m_thickness_program.setUniformValue("height", height);
        renderSpriteWithShader(m_thickness_program);

        glDisable(GL_BLEND);
        m_thickness_program.release();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Quad quad;
        quad.setTexture(m_thickness_tex[0]);
        quad.draw();
    }
    void Render::resetMatrix()
    {
        m_modelview_mat.setToIdentity();
        m_modelview_mat.translate(0, 0, -9);

        m_rotate_mat.setToIdentity();
    }

    void Render::moveX(float x)
    {
        m_modelview_mat.translate(x, 0, 0);
    }

    void Render::moveZ(float x)
    {
        m_modelview_mat.translate(0, 0, x);
    }

    void Render::rotateY(float x)
    {
        m_rotate_mat.rotate(x, 0, 1, 0);
    }

    void Render::rotateX(float x)
    {
        m_rotate_mat.rotate(x, 1, 0, 0);
    }

    void Render::setWindowDimensions(GLuint width, GLuint height)
    {
        window_width = width;
        window_height = height;
    }
    }

}


