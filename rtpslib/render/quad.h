#ifndef _RENDER_QUAD_H
#define _RENDER_QUAD_H

#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include <vector>

namespace rtps
{
    namespace render
    {

        class Quad : protected QOpenGLFunctions_4_3_Core
        {
        public:
            Quad(float x0 = -1, float y0 = -1, float x1= 1,float y1 = 1);
            void setTexture(QOpenGLTexture *texture) { m_texture = texture->textureId(); }
            void setTexture(GLuint texture) { m_texture = texture;  };
            void draw();
            void drawMesh(QOpenGLShaderProgram& program);

        private:
            float m_x0, m_y0, m_x1, m_y1;
            QOpenGLVertexArrayObject m_vao;
            QOpenGLBuffer m_pos_vbo;
            QOpenGLBuffer m_tex_vbo;
            QOpenGLShaderProgram m_program;
            GLuint m_texture;
            
        };
    }
}


#endif
