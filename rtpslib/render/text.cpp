#include <render/text.h>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLVersionFunctions>
#include <QImage>
#include <QOpenGLTexture>

namespace rtps
{
    namespace render
    {
        std::auto_ptr<QOpenGLTexture> Text::texture;
        Text::Text(const std::string& text)
            : m_pos_vbo(QOpenGLBuffer::VertexBuffer)
            , m_text(text)
        {
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
            m_program.setUniformValue("cell_size", 1.0f/16, (300.0f/384) / 6);
            m_program.setUniformValue("cell_offset", 0.5/256.0, 0.5/256.0);
        }

        void Text::initBuffer()
        {
            m_vao.create();
            m_vao.bind();
            m_pos_vbo.create();
            m_pos_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
            m_pos_vbo.bind();
            m_pos_vbo.allocate(m_text.c_str(), m_text.size() * sizeof(char));
            m_program.enableAttributeArray(0);
            m_gl_funcs->glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, 0, 0);
            
            m_vao.release();

        }

        void Text::initTexture()
        {
            if(texture.get())
                return;
            //m_gl_funcs->glGenTextures(1, &texture_id);
            //m_gl_funcs->glBindTexture(GL_TEXTURE_2D, texture_id);
            //m_gl_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            //m_gl_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            //m_gl_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            //m_gl_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            texture = std::auto_ptr<QOpenGLTexture>(new QOpenGLTexture(QImage("./bin/textures/font.png").mirrored()));
            texture->setMinificationFilter(QOpenGLTexture::Linear);
            texture->setMagnificationFilter(QOpenGLTexture::Linear);
            texture->setWrapMode(QOpenGLTexture::ClampToEdge);
        }

        Text::~Text()
        {
        }

        void Text::draw(float x, float y, int w, int h,  const float4& color)
        {
            m_program.bind();
            texture->bind();
            m_program.setUniformValue("text_color", color.x, color.y, color.z);
            m_program.setUniformValue("render_size", 0.75 * 16 / w * 2, 0.75 * 33.33 / h * 2);
            m_program.setUniformValue("render_origin", x, y);
            
            m_gl_funcs->glEnable(GL_BLEND);
            m_gl_funcs->glDisable(GL_DEPTH_TEST);
            m_vao.bind();
            m_gl_funcs->glDrawArrays(GL_POINTS, 0, m_text.size());
            texture->release();
            m_program.release();

        }
    }

}
