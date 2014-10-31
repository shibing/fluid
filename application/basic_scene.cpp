#include "basic_scene.h"
#include "opengl_window.h"

#include <RTPS.h>
#include <RTPSettings.h>
#include <system/System.h>

#include <QObject>
#include <QOpenGLContext>
#include <QKeyEvent>

using namespace rtps;

BasicScene::BasicScene()
    :max_num(100000) ,
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
    ps->getRender()->setOpenGLFunctions(m_window->getOpenGLFunctions());
    ps->getSystem()->setOpenGLFunctions(m_window->getOpenGLFunctions());

}

void BasicScene::update(float t)
{
    Q_UNUSED(t);
    ps->update();
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

bool BasicScene::keyPress(QKeyEvent *event)
{
    int n = 2048;
    float4 min;  
    float4 max;
    switch(event->key()) {
        case Qt::Key_R: 
            min = float4(-0.5, -0.5, 0.7, 1.0);
            max = float4( 0.5,  0.5, 1.5, 1.0);
            ps->system->addBox(n, min, max, false, float4(1.0, 1.0, 1.0, 1.0));
            return true;
        default:
            return false;
    }
}

void BasicScene::renderOverlay()
{
    glDisable(GL_DEPTH_TEST);

}
