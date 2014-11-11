#ifndef _RENDER_CUBE_H
#define _RENDER_CUBE_H

#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMatrix4x4>

namespace rtps
{
    namespace render
    {
        class Cube : protected QOpenGLFunctions_4_3_Core
        {
        public:
            Cube();
            void draw(const QMatrix4x4& mat);
        private:
            void initCubeTexture();
        private:
            QOpenGLVertexArrayObject m_vao;

            QOpenGLBuffer m_pos_vbo;
            QOpenGLBuffer m_index_vbo;
            GLuint m_texture;
            QOpenGLShaderProgram m_program;
        };
    }
}

#endif
