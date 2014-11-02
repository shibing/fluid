#include <render/quad.h>
#include <QOpenGLTexture>

namespace rtps
{
    namespace render
    {
        Quad::Quad(float x0, float y0, float x1, float y1)
            : m_pos_vbo(QOpenGLBuffer::VertexBuffer)
            , m_tex_vbo(QOpenGLBuffer::VertexBuffer)
            , m_x0(x0)
            , m_y0(y0)
            , m_x1(x1)
            , m_y1(y1)
        {
            initializeOpenGLFunctions();
            m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, "./bin/shaders/quad.vert");
            m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, "./bin/shaders/quad.frag");
            m_program.link();
            float data[] = {        
                -1.0, -1.0,
                -1.0,  1.0,
                 1.0, -1.0,
                 1.0,  1.0,
            };

            float tex_coord[] = {
                    0, 0,
                    0, 1,
                    1, 0,
                    1, 1,
            };
    
            m_vao.create();
            m_vao.bind();
            m_pos_vbo.create();
            m_pos_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
            m_pos_vbo.bind();
            m_pos_vbo.allocate(data, sizeof(data));
            m_program.enableAttributeArray(0);
            m_program.setAttributeBuffer(0, GL_FLOAT, 0, 2);

            m_tex_vbo.create();
            m_tex_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
            m_tex_vbo.bind();
            m_tex_vbo.allocate(tex_coord, sizeof(tex_coord));
            m_program.enableAttributeArray(1);
            m_program.setAttributeBuffer(1, GL_FLOAT, 0, 2);

            m_vao.release();

        }

        void Quad::draw()
        {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            m_program.bind();
            glActiveTexture(GL_TEXTURE0 + 80);
            m_texture->bind();
            m_program.setUniformValue("sampler", GLuint(80));
            m_vao.bind();
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            m_vao.release();
            m_texture->release();
            m_program.release();
        }

    }
}
