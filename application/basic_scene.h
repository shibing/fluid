#ifndef BASIC_SCENE_H
#define BASIC_SCENE_H

#include "abstract_scene.h"


#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

namespace rtps {
class Domain;
class RTPSettings;
class RTPS;
}


class BasicScene : public AbstractScene 
{
public:
    BasicScene();

    virtual void initialize();
    virtual void update(float t = 0);
    virtual void render();
    virtual void resize(int width, int height);

private:
    int max_num;
    int dt;
    int window_width;
    int window_height;
    rtps::Domain *domain;
    rtps::RTPSettings *settings;
    rtps::RTPS *ps;
    
};

#endif // BASICUSAGESCENE_H
