
#ifndef _RENDER_TEXT_H
#define _RENDER_TEXT_H

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <string>
#include <structs.h>

#include <memory>

class QOpenGLFunctions_4_3_Core;
class QOpenGLTexture;

namespace rtps
{
namespace render
{

class Text
{
public:
    Text(const std::string& text);
    void setText(const std::string& text);
    void draw(float x, float y, int w, int h, const float4& color = float4(1.0, 1.0, 1.0, 1.0));
    void setOpenGLFunctions(QOpenGLFunctions_4_3_Core *funcs) { m_gl_funcs = funcs; }
    void initTexture();
    void initProgram();
    void initBuffer();

    ~Text();

private:

    std::string m_text;

    QOpenGLShaderProgram m_program;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_pos_vbo;
    QOpenGLFunctions_4_3_Core *m_gl_funcs;

    static std::auto_ptr<QOpenGLTexture> texture;
};

}
}

#endif
