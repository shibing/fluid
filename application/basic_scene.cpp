#include "basic_scene.h"
#include "opengl_window.h"

#include <RTPS.h>
#include <RTPSettings.h>
#include <system/System.h>

#include <QObject>
#include <QOpenGLContext>

using namespace rtps;

BasicScene::BasicScene()
    :max_num(10000) ,
     dt(0.003)
{
    max_num = nlpo2(max_num);
}

void BasicScene::initialize()
{

    Domain *domain = new Domain(float4(-2.5, -2.5, -2.5, 1), float4(2.5, 2.5, 2.5, 1));
    settings = new RTPSettings(RTPSettings::SPH, max_num, dt, domain); 
    settings->SetSetting("rtps_path", "./bin");
    settings->setRenderType(RTPSettings::RENDER);
    settings->setRadiusScale(0.4);
    settings->setBlurScale(1.0);
    settings->setUseGLSL(1);
    settings->SetSetting("sub_intervals", 1);
    settings->SetSetting("render_texture", "firejet_blast.png");
    settings->SetSetting("render_frag_shader", "sprite_tex_frag.glsl");
    settings->SetSetting("render_use_alpha", true);
    settings->SetSetting("render_alpha_function", "add");
    settings->SetSetting("lt_increment", -.00);
    settings->SetSetting("lt_cl", "lifetime.cl");
    ps = new RTPS(settings);
}

void BasicScene::update(float t)
{
    Q_UNUSED(t);
}

void BasicScene::render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, m_window->width(), m_window->height());
    ps->getRender()->setWindowDimensions(m_window->width(), m_window->height());
    ps->render();
}

void BasicScene::resize(int width, int height)
{
     window_width = width;
     window_height = height;
}


