
#ifndef _RENDER_TEXT_H
#define _RENDER_TEXT_H

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <string>
#include <structs.h>
#include <QOpenGLFunctions_4_3_Core>
#include <memory>

class QOpenGLTexture;

namespace rtps
{
namespace render
{

class Text : protected QOpenGLFunctions_4_3_Core
{
public:
    Text();
    void draw(const std::string& text, float x, float y, int w, int h, const float4& color = float4(1.0, 1.0, 1.0, 1.0));
    void initTexture();
    void initProgram();
    void initBuffer();

    ~Text();

public:
    static void releaseResource();

private:

    std::string m_text;

    QOpenGLShaderProgram m_program;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_pos_vbo;

    static QOpenGLTexture*  texture;
};

}
}

#endif
