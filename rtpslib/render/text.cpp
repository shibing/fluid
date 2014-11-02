#include <iostream>
#include <render/text.h>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLVersionFunctions>
#include <QImage>
#include <QOpenGLTexture>

namespace rtps
{
    namespace render
    {
        QOpenGLTexture *Text::texture = 0;
        Text::Text()
            : m_pos_vbo(QOpenGLBuffer::VertexBuffer)
        {
            initializeOpenGLFunctions();
            initTexture();
            initProgram();
            initBuffer();
        }

        void Text::initProgram()
        {
            m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, "./bin/shaders/simple_text.vert");
            m_program.addShaderFromSourceFile(QOpenGLShader::Geometry, "./bin/shaders/simple_text.geom");
            m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, "./bin/shaders/simple_text.frag");
            m_program.link();
        }

        void Text::initBuffer()
        {
            m_program.bind();
            m_vao.create();
            m_vao.bind();
            m_pos_vbo.create();
            m_pos_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
            m_pos_vbo.bind();
            glEnableVertexAttribArray(0);
            glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, 0, 0);
            m_vao.release();
        }

        void Text::initTexture()
        {
            if(texture)
                return;

            texture = new QOpenGLTexture(QImage("./bin/textures/font.png"));
            texture->setMinificationFilter(QOpenGLTexture::Linear);
            texture->setMagnificationFilter(QOpenGLTexture::Linear);
            texture->setWrapMode(QOpenGLTexture::ClampToEdge);
        }

        Text::~Text()
        {
        }

        void Text::draw(const std::string& text, float x, float y, int w, int h,  const float4& color)
        {
            m_pos_vbo.bind();
            m_pos_vbo.allocate(text.c_str(), text.size() * sizeof(char));

            m_program.bind();
            glActiveTexture(GL_TEXTURE0);
            texture->bind();

            m_program.setUniformValue("text_color", color.x, color.y, color.z);
            m_program.setUniformValue("cell_size", 1.0f/16, (300.0f/384) / 6);
            m_program.setUniformValue("cell_offset", 0.5/256.0, 0.5/256.0);
            m_program.setUniformValue("sampler", (GLuint)0);
            m_program.setUniformValue("render_size", 0.45 * 16 / w , 0.45 * 33.33 / h );
            m_program.setUniformValue("render_origin", x, y);
            
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            m_vao.bind();
            glDrawArrays(GL_POINTS, 0, text.size());

            texture->release();
            m_program.release();
        }

        void Text::releaseResource()
        {
            delete texture;
            texture  = 0;
        }
    }

}
