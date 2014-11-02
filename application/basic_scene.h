#ifndef BASIC_SCENE_H
#define BASIC_SCENE_H

#include "abstract_scene.h"

#include <structs.h>

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

class QKeyEvent;

namespace rtps {
class Domain;
class RTPSettings;
class RTPS;

namespace render {
class Text;
}

}


class BasicScene : public AbstractScene 
{
public:
    BasicScene();

    virtual void initialize();
    virtual void update(float t = 0);
    virtual void render();
    virtual void resize(int width, int height);
    virtual bool keyPress(QKeyEvent * event) ;
    virtual bool mousePress(QMouseEvent *);
    virtual bool mouseMove(QMouseEvent *);

    void renderOverlay();

private:
    template<typename T>
    void renderKeyValue(const std::string& key, const T&value, float x, float y, const rtps::float4& color, rtps::render::Text& text);

private:
    int max_num;
    float dt;
    int window_width;
    int window_height;
    rtps::Domain *domain;
    rtps::RTPSettings *settings;
    rtps::RTPS *ps;

    int mouse_pos_x;
    int mouse_pos_y;

    bool show_help;
    bool show_info;
};

#endif // BASICUSAGESCENE_H
