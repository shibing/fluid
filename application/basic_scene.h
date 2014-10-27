#ifndef BASIC_SCENE_H
#define BASIC_SCENE_H

#include "abstract_scene.h"

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

class BasicScene : public AbstractScene 
{
public:
    BasicScene();

    virtual void initialize();
    virtual void update(float t = 0);
    virtual void render();
    virtual void resize(int width, int height);

private:
    QOpenGLShaderProgram mShaderProgram;
    QOpenGLVertexArrayObject mVAO;
    QOpenGLBuffer mVertexPositionBuffer;
    QOpenGLBuffer mVertexColorBuffer;

    void prepareShaderProgram();
    void prepareVertexBuffers();
};

#endif // BASICUSAGESCENE_H
