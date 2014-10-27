#include "basic_scene.h"

#include <QObject>
#include <QOpenGLContext>

BasicScene::BasicScene()
    : mShaderProgram(),
      mVertexPositionBuffer(QOpenGLBuffer::VertexBuffer),
      mVertexColorBuffer(QOpenGLBuffer::VertexBuffer)
{
}

void BasicScene::initialize()
{
    prepareShaderProgram();
    prepareVertexBuffers();
}

void BasicScene::update(float t)
{
    Q_UNUSED(t);
}

void BasicScene::render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    mShaderProgram.bind();
    mVAO.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    mShaderProgram.release();
}

void BasicScene::resize(int width, int height)
{
     glViewport(0, 0, width, height);
}

void BasicScene::prepareShaderProgram()
{
    if (!mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "./bin/shaders/phong.vert"))
    {
//        qCritical() << "error";
    }
    if (!mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "./bin/shaders/phong.frag"))
    {
 //       qCritical() << "error";
    }
    if (!mShaderProgram.link())
    {
  //      qCritical() << "error";
    }
}

void BasicScene::prepareVertexBuffers()
{
    float positionData[] = {
        -0.8f, -0.8f, 0.0f,
         0.8f, -0.8f, 0.0f,
         0.0f,  0.8f, 0.0f
    };
    float colorData[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    mVAO.create();
    mVAO.bind();

    mVertexPositionBuffer.create();
    mVertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVertexPositionBuffer.bind();
    mVertexPositionBuffer.allocate(positionData, 3 * 3 * sizeof(float));

    mVertexColorBuffer.create();
    mVertexColorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVertexColorBuffer.bind();
    mVertexColorBuffer.allocate(colorData, 3 * 3 * sizeof(float));

    mShaderProgram.bind();

    mVertexPositionBuffer.bind();
    mShaderProgram.enableAttributeArray("vertexPosition");
    mShaderProgram.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    mVertexColorBuffer.bind();
    mShaderProgram.enableAttributeArray("vertexColor");
    mShaderProgram.setAttributeBuffer("vertexColor", GL_FLOAT, 0, 3);
}
