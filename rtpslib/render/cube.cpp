#include "cube.h"

namespace rtps
{
    namespace render
    {
        Cube::Cube()
            :m_pos_vbo(QOpenGLBuffer::VertexBuffer)
            ,m_index_vbo(QOpenGLBuffer::IndexBuffer)
        {
            initializeOpenGLFunctions();
            m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, "./bin/shaders/cube.vs");
            m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, "./bin/shaders/cube.fs");
            m_program.link();

            static GLfloat cube_vertices[] = {
                -1.0,  1.0,  1.0,
                -1.0, -1.0,  1.0,
                 1.0, -1.0,  1.0,
                 1.0,  1.0,  1.0,
                -1.0,  1.0, -1.0,
                -1.0, -1.0, -1.0,
                 1.0, -1.0, -1.0,
                 1.0,  1.0, -1.0,
            };

            static GLushort cube_indices[] = {
                1, 0, 2,
                0, 3, 2,
                6, 3, 2,
                7, 6, 3,
                4, 5, 6,
                6, 7, 4,
                4, 0, 5,
                5, 0, 1,
                0, 3, 4,
                3, 7, 4,
                6, 5, 2,
                5, 1, 2,
            };

            m_vao.create();
            m_vao.bind();
            m_pos_vbo.create();
            m_pos_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
            m_pos_vbo.bind();
            m_pos_vbo.allocate(cube_vertices, sizeof(cube_vertices));
            m_program.enableAttributeArray(0);
            m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3);

            m_index_vbo.create();
            m_index_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
            m_index_vbo.bind();
            m_index_vbo.allocate(cube_indices, sizeof(cube_indices));
            
            m_vao.release();
            initCubeTexture();
        }

        void Cube::initCubeTexture()
        {
            QImage xpos("./bin/textures/xpos.png");
            QImage xneg("./bin/textures/xneg.png");
            QImage ypos("./bin/textures/ypos.png");
            QImage yneg("./bin/textures/yneg.png");
            QImage zpos("./bin/textures/zpos.png");
            QImage zneg("./bin/textures/zneg.png");
            int width = xpos.width();
            int height = xpos.height();

            glGenTextures(1, &m_texture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, xpos.bits()); 
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, xneg.bits()); 
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, ypos.bits()); 
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, yneg.bits()); 
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, zpos.bits()); 
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, zneg.bits()); 
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        }

        void Cube::draw(const QMatrix4x4& mat)
        {
            m_program.bind();
            glActiveTexture(GL_TEXTURE0 + 33);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);
            m_program.setUniformValue("mat", mat);
            m_program.setUniformValue("sampler", (GLuint)33);

            GLint old_cull_face_mode;
            glGetIntegerv(GL_CULL_FACE_MODE, &old_cull_face_mode);
            GLint old_depth_func_mode;
            glGetIntegerv(GL_DEPTH_FUNC, &old_depth_func_mode);

            glEnable(GL_DEPTH_TEST);
            glCullFace(GL_FRONT);
            glDepthFunc(GL_LEQUAL);

            m_vao.bind();
            glDrawElements(GL_TRIANGLES, 2 * 6 * 3, GL_UNSIGNED_SHORT, 0); 
            m_program.release();
            m_vao.release();

            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            glCullFace(old_cull_face_mode);
            glDisable(GL_DEPTH_TEST);
            glDepthFunc(old_depth_func_mode);

        }

    }
}
