
#include <QOpenGLFunctions_4_3_Core>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include <RTPSettings.h>

#include <render/Render.h>

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

        initShaderProgram();
        initBoxBuffer();
        initParticleBuffer();
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
        }
    }

    void Render::renderBox()
    {
        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");
        m_perspective_mat.setToIdentity();
        m_perspective_mat.perspective(60.0f, width/(height * 1.0f), 0.1, 1000.0f);

        m_opengl_funcs-> glEnable(GL_DEPTH_TEST);
        m_basic_program.bind();
        GLuint uniform_matrix = m_basic_program.uniformLocation("matrix");
        m_basic_program.setUniformValue(uniform_matrix, m_perspective_mat * m_modelview_mat * m_rotate_mat);
        m_box_vao.bind();
        m_opengl_funcs->glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        m_basic_program.release();
    }

    void Render::renderFluidAsPoint()
    {

        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");
        m_perspective_mat.setToIdentity();
        m_perspective_mat.perspective(60.0f, width/(height * 1.0f), 0.1, 1000.0f);
        
        m_opengl_funcs->glPointSize(5.0f);
        m_opengl_funcs->glEnable(GL_DEPTH_TEST);

        m_particle_program.bind();
        m_particle_vao.bind();

        GLuint uniform_matrix = m_basic_program.uniformLocation("matrix");
        m_particle_program.setUniformValue(uniform_matrix, m_perspective_mat * m_modelview_mat * m_rotate_mat);
        m_opengl_funcs->glDrawArrays(GL_POINTS, 0, m_num);
        m_opengl_funcs->glFinish(); 

        m_particle_vao.release();
        m_particle_program.release();
    }

    void Render::renderFluidAsSphere()
    {

        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");
        m_perspective_mat.setToIdentity();
        m_perspective_mat.perspective(60.0f, width/(height * 1.0f), 0.1, 1000.0f);
        
        //m_opengl_funcs->glPointSize(15.0f);
        m_opengl_funcs->glEnable(GL_PROGRAM_POINT_SIZE);
        m_opengl_funcs->glEnable(GL_DEPTH_TEST);

        m_sphere_program.bind();
        m_particle_vao.bind();

        m_sphere_program.setUniformValue("modelview_mat", m_modelview_mat * m_rotate_mat);
        m_sphere_program.setUniformValue("projection_mat" , m_perspective_mat);
        m_sphere_program.setUniformValue("sphere_radius", m_settings->GetSettingAs<float>("Spacing"));
        m_sphere_program.setUniformValue("near", 0.1f);
        m_sphere_program.setUniformValue("far", 1000.0f);

        m_opengl_funcs->glDrawArrays(GL_POINTS, 0, m_num);
        m_opengl_funcs->glFinish(); 

        m_opengl_funcs->glDisable(GL_PROGRAM_POINT_SIZE);
        m_opengl_funcs->glDisable(GL_DEPTH_TEST);

        m_particle_vao.release();
        m_sphere_program.release();
    }
    void Render::resetMatrix()
    {
        m_modelview_mat.setToIdentity();
        m_modelview_mat.translate(0, 0, -10);

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


