
#include <QOpenGLFunctions_4_3_Core>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <QOpenGLTexture>
#include <fstream>

#include <RTPSettings.h>

#include <render/Render.h>
#include <render/quad.h>
#include <render/cube.h>

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
        initProgramAndKernel();
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

        assert(m_show_depth_program.addShaderFromSourceFile(QOpenGLShader::Vertex, (shader_source_dir + "/quad.vert").c_str()));
        assert(m_show_depth_program.addShaderFromSourceFile(QOpenGLShader::Fragment, (shader_source_dir + "/show_depth.frag").c_str()));
        assert(m_show_depth_program.link());

        assert(m_compose_program.addShaderFromSourceFile(QOpenGLShader::Vertex, (shader_source_dir + "/quad.vert").c_str()));
        assert(m_compose_program.addShaderFromSourceFile(QOpenGLShader::Fragment, (shader_source_dir + "/compose.frag").c_str()));
        assert(m_compose_program.link());

    }

    void Render::initFramebufferObject()
    {
        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");
        glGenTextures(2, &m_depth_tex[0]);
        //init cl buffer here
        int err;
        for(int i = 0; i < 2; ++i) {
            glBindTexture(GL_TEXTURE_2D, m_depth_tex[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
            glBindTexture(GL_TEXTURE_2D, 0);

            m_depth_cl[i] = new cl::Image2DGL(m_cli->context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, m_depth_tex[i], &err);
            if(err != CL_SUCCESS) {
                std::cout << "can not create cl texture from opengl texture" << std::endl;
            }
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

        glGenTextures(1, &m_background_tex);
        glBindTexture(GL_TEXTURE_2D, m_background_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);

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

    void Render::initProgramAndKernel()
    {
        int err;
        std::string path = m_settings->GetSettingAs<std::string>("rtps_path");
        path += "/cl_common/curvature_flow.cl";
        std::ifstream file(path.c_str());
        std::string prog(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
        cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length() + 1));
        m_program = new cl::Program(m_cli->context, source);
        err =  m_program->build(m_cli->devices);
        m_curvature_flow_kernel = new cl::Kernel(*m_program, "curvature_flow", &err);
        if(err != CL_SUCCESS)
            std::cerr << "can't create kernel" << std::endl;
    }

    void Render::smoothDepth()
    {
        int iteration_count = 100;
        float dt = 0.005;
        float z_contrib = 40;
        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");
        glFinish();
        cl::Event event;
        int err;
        std::vector<cl::Memory> buffers;
        buffers.push_back(*m_depth_cl[0]);
        buffers.push_back(*m_depth_cl[1]);
        err = m_cli->queue.enqueueAcquireGLObjects(&buffers, NULL, &event);
        m_cli->queue.finish();
        m_curvature_flow_kernel->setArg(2, dt);
        m_curvature_flow_kernel->setArg(3, z_contrib);
        m_curvature_flow_kernel->setArg(4, width);
        m_curvature_flow_kernel->setArg(5, height);
        m_curvature_flow_kernel->setArg(6, m_perspective_mat.data()[0]);
        m_curvature_flow_kernel->setArg(7, m_perspective_mat.data()[5]);

        for(int i = 0; i < iteration_count; ++i) {
            m_curvature_flow_kernel->setArg(0, *m_depth_cl[0]);
            m_curvature_flow_kernel->setArg(1, *m_depth_cl[1]);
            err = m_cli->queue.enqueueNDRangeKernel(*m_curvature_flow_kernel, cl::NullRange, cl::NDRange(width, height), cl::NullRange, NULL, &event);
            m_cli->queue.finish();
            std::swap(m_depth_cl[0], m_depth_cl[1]);
        }
        err = m_cli->queue.enqueueReleaseGLObjects(&buffers, NULL, &event);
        m_cli->queue.finish();
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
        glEnable(GL_PROGRAM_POINT_SIZE);
        m_particle_vao.bind();
        glDrawArrays(GL_POINTS, 0, m_num);
        glDisable(GL_PROGRAM_POINT_SIZE);
        m_particle_vao.release();
        
    }

    void Render::renderBox()
    {
        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");
        m_perspective_mat.setToIdentity();
        m_perspective_mat.perspective(60.0f, width/(height * 1.0f), 1, 100.0f);

        glEnable(GL_DEPTH_TEST);
        m_basic_program.bind();
        GLuint uniform_matrix = m_basic_program.uniformLocation("matrix");
        m_basic_program.setUniformValue(uniform_matrix, m_perspective_mat * m_translate_mat * m_rotate_mat);
        m_box_vao.bind();
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        m_basic_program.release();
    }

    void Render::renderFluidAsPoint()
    {
        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");
        m_perspective_mat.setToIdentity();
        m_perspective_mat.perspective(60.0f, width/(height * 1.0f), 1.0, 100.0f);

        m_particle_program.bind();
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_DEPTH_TEST);
        m_particle_vao.bind();
        GLuint uniform_matrix = m_basic_program.uniformLocation("matrix");
        m_particle_program.setUniformValue(uniform_matrix, m_perspective_mat * m_translate_mat * m_rotate_mat);
        glDrawArrays(GL_POINTS, 0, m_num);
        glDisable(GL_PROGRAM_POINT_SIZE);
        m_particle_vao.release();
        m_particle_program.release();

        m_cube.draw(m_perspective_mat * m_rotate_mat);
    }

    void Render::renderFluidAsSphere()
    {
        int width = m_settings->GetSettingAs<int>("window_width");
        int height = m_settings->GetSettingAs<int>("window_height");

        m_sphere_program.bind();

        m_sphere_program.setUniformValue("modelview_mat", m_translate_mat * m_rotate_mat);
        m_sphere_program.setUniformValue("projection_mat" , m_perspective_mat);
        m_sphere_program.setUniformValue("sphere_radius", m_settings->GetSettingAs<float>("Spacing"));
        m_sphere_program.setUniformValue("near", 1.0f);
        m_sphere_program.setUniformValue("far", 100.0f);
        m_sphere_program.setUniformValue("width", width);
        m_sphere_program.setUniformValue("height", height);

        glViewport(0, 0, width, height);
        glEnable(GL_DEPTH_TEST);
        renderSpriteWithShader(m_sphere_program);
        glDisable(GL_DEPTH_TEST);

        m_sphere_program.release();
        m_cube.draw(m_perspective_mat * m_rotate_mat);
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
        m_depth_program.setUniformValue("modelview_mat", m_translate_mat * m_rotate_mat);
        m_depth_program.setUniformValue("projection_mat" , m_perspective_mat);
        m_depth_program.setUniformValue("sphere_radius", m_settings->GetSettingAs<float>("Spacing"));
        m_depth_program.setUniformValue("near", 0.1f);
        m_depth_program.setUniformValue("far", 1000.0f);
        m_depth_program.setUniformValue("width", width);
        m_depth_program.setUniformValue("height", height);

        renderSpriteWithShader(m_depth_program);

        glDisable(GL_DEPTH_TEST);

        m_depth_program.release();

        smoothDepth();

        //draw thickness
        m_thickness_program.bind();

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_thickness_tex[0], 0);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glViewport(0, 0, width, height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_thickness_program.bind();
        m_thickness_program.setUniformValue("modelview_mat", m_translate_mat * m_rotate_mat);
        m_thickness_program.setUniformValue("projection_mat" , m_perspective_mat);
        m_thickness_program.setUniformValue("sphere_radius", m_settings->GetSettingAs<float>("Spacing"));
        m_thickness_program.setUniformValue("num", (int)m_settings->GetSettingAs<int>("Number of Particles"));
        m_thickness_program.setUniformValue("near",1.0f);
        m_thickness_program.setUniformValue("far", 100.0f);
        m_thickness_program.setUniformValue("width", width);
        m_thickness_program.setUniformValue("height", height);
        renderSpriteWithShader(m_thickness_program);

        glDisable(GL_BLEND);
        m_thickness_program.release();

        //draw background
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_background_tex, 0);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, width ,height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_cube.draw(m_perspective_mat * m_rotate_mat);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //final image
        Quad quad;
        m_compose_program.bind();
        m_compose_program.setUniformValue("width", width);
        m_compose_program.setUniformValue("height", height);
        m_compose_program.setUniformValue("projection_mat", m_perspective_mat);
        m_compose_program.setUniformValue("inverse_proj", m_perspective_mat.inverted());
        m_compose_program.setUniformValue("inverse_modelview", (m_translate_mat * m_rotate_mat).inverted());
        m_compose_program.setUniformValue("texel_size", 1.0 / width, 1.0 / height);
        m_compose_program.setUniformValue("sphere_radius", m_settings->GetSettingAs<float>("Spacing_No_Scale"));
        m_compose_program.setUniformValue("particle_num", m_settings->GetSettingAs<int>("Number of Particles"));
        glViewport(0, 0, width, height);
        glActiveTexture(GL_TEXTURE0 + 21);
        glBindTexture(GL_TEXTURE_2D, m_depth_tex[0]);
        m_compose_program.setUniformValue("depth_tex", GLuint(21));
        glActiveTexture(GL_TEXTURE0 + 22);
        glBindTexture(GL_TEXTURE_2D, m_thickness_tex[0]);
        m_compose_program.setUniformValue("thickness_tex", GLuint(22));
        glActiveTexture(GL_TEXTURE0 + 23);
        glBindTexture(GL_TEXTURE_2D, m_background_tex);
        m_compose_program.setUniformValue("background_tex", GLuint(23));
        glActiveTexture(GL_TEXTURE0 + 24);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_cube.getTexture());
        m_compose_program.setUniformValue("cube_map_tex", GLuint(24));
        quad.drawMesh(m_compose_program);

    }
    void Render::resetMatrix()
    {
        m_translate_mat.setToIdentity();
        m_translate_mat.translate(0, 3, -24);
        m_rotate_mat.setToIdentity();
    }

    void Render::moveX(float x)
    {
        m_translate_mat.translate(x, 0, 0);
    }

    void Render::moveZ(float x)
    {
        m_translate_mat.translate(0, 0, x);
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


