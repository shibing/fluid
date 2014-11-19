#include "basic_scene.h"
#include "opengl_window.h"

#include <RTPS.h>
#include <RTPSettings.h>
#include <system/System.h>
#include <render/text.h>
#include <render/Render.h>
#include <render/quad.h>

#include <QObject>
#include <QOpenGLContext>
#include <QKeyEvent>
#include <QGuiApplication>

using namespace rtps;

#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_4_3_Core>
#include <QMouseEvent>

QOpenGLTexture *texture;

BasicScene::BasicScene()
    : max_num(200000) 
    , dt(0.002)
    , show_help(true)
    , show_info(true)

{
    max_num = nlpo2(max_num);
}

void BasicScene::initialize()
{
    Domain *domain = new Domain(float4(-3.0, -6.0, -3.0, 1), float4(3.0, 6.0, 3.0, 1));
    settings = new RTPSettings(RTPSettings::SPH, max_num, dt, domain); 
    settings->SetSetting("rtps_path", "./bin");
    settings->setRenderType(RTPSettings::RENDER);
    settings->setRadiusScale(0.4);
    settings->setBlurScale(1.0);
    settings->setUseGLSL(1);
    settings->SetSetting("sub_intervals", 4);
    settings->SetSetting("window_width", m_window->getWidth());
    settings->SetSetting("window_height", m_window->getHeight());
    ps = new RTPS(settings);
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
    glDisable(GL_BLEND);
    glViewport(0, 0, m_window->width(), m_window->height());
    ps->render();

    renderOverlay();
}

void BasicScene::resize(int width, int height)
{
     settings->SetSetting("window_width", m_window->getWidth());
     settings->SetSetting("window_height", m_window->getHeight());
     window_width = width;
     window_height = height;
}

bool BasicScene::keyPress(QKeyEvent *event)
{
    int n;
    float4 min;  
    float4 max;
    float4 center;
    float4 velocity;
    static int h = 0; //fluid height

    static render::Render::RenderType type[3] = {render::Render::POINT, render::Render::SPHERE, render::Render::SURFACE}; 

    static int i = 0;

    switch(event->key()) {
        case Qt::Key_I:
            show_info = !show_info;
            return true;
        case Qt::Key_H:
            if(event->modifiers() & Qt::ShiftModifier) {
                show_help = !show_help;
            }
            else {
                center =  float4(-6.0, 1.8, 0, 1.0);
                velocity = float4(2.2, 0.0, 0.0, 0);
                ps->system->addHose(System::OIL, 50000, center, velocity, 6, float4(0.0, 1.0, 0.0, 1.0));
            }
            return true;
        case Qt::Key_9:
            center = float4(-7.0, 1.5, 0, 1.0);
            velocity = float4(1.2, 0.0, 0.0, 0);
            ps->system->addHose(System::OIL, 50000, center, velocity, 6, float4(0.0, 1.0, 0.0, 1.0));

            center =  float4(7.0, 1.5, 0, 1.0);
            velocity = float4(-1.2, 0.0, 0.0, 0);
            ps->system->addHose(System::WATER, 50000, center, velocity, 6, float4(1.0, 0.0, 0.0, 1.0));
            return true;
        case Qt::Key_E:
            n = 163840;
            min = float4(-2.9, -2.2 + i * 1, -2.9, 1.0);
            max = float4( 2.9,  1.2 + i * 1,  2.9, 1.0);
            ++i;
            ps->system->addBox(System::WATER, n, min, max, float4(1.0, 0.0, 0.0, 1.0));
            return true;
        case Qt::Key_O:
            n = 163840;
            min = float4(-2.9,  -2.2 + i * 1, -2.9, 1.0);
            max = float4( 2.9,   1.2 + i * 1,  2.9, 1.0);
            ++i;
            ps->system->addBox(System::OIL, n, min, max, float4(0.0, 1.0, 0.0, 1.0));
            return true;
        case Qt::Key_M:
            n = 163840;
            min = float4(-8.9, -3.2, -3.0, 1.0);
            max = float4(-5.0, -0.2,  3.0, 1.0);
            ps->system->addBox(System::OIL, n, min, max, float4(0.0, 1.0, 0.0, 1.0));

            min = float4( 5.0, -3.2, -3.0, 1.0);
            max = float4( 8.9, -0.2,  3.0, 1.0);
            ps->system->addBox(System::WATER, n, min, max, float4(1.0, 0.0, 0.0, 1.0));
            return true;
        case Qt::Key_B:
            ps->system->addBunny(System::WATER, float4(0, 0, 0, 0));
            return true;
        case Qt::Key_8:
            ps->system->addBunny(System::OIL, float4(0, 0, 0, 0));
            return true;
        case Qt::Key_T:
            ps->system->setPaused();
            return true;
        case Qt::Key_L:
            ps->system->setPush();
            return true;
        case Qt::Key_Q:
        case Qt::Key_Escape:
            rtps::render::Text::releaseResource();
            QGuiApplication::exit();
            return true;
        case Qt::Key_W:
            ps->getRender()->moveZ(0.05);
            return true;
        case Qt::Key_S:
            ps->getRender()->moveZ(-0.05);
            return true;
        case Qt::Key_A:
            ps->getRender()->moveX(0.05);
            return true;
        case Qt::Key_D:
            ps->getRender()->moveX(-0.05);
            return true;
        case Qt::Key_Space:
            ps->getRender()->resetMatrix();
            return true;
        case Qt::Key_P:
            i = (i + 1) % 3;
            ps->getRender()->setRenderType(type[i]);
            return true;
        default:
            return false;
    }
}

bool BasicScene::mousePress(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton) {
        mouse_pos_x = event->x();
        mouse_pos_y = event->y();
        return true;
    }
    return false;

}

bool BasicScene::mouseMove(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton) {
        int new_x = event->x();
        int new_y = event->y();
        ps->getRender()->rotateY((new_x - mouse_pos_x) * 0.2);
        ps->getRender()->rotateX((new_y - mouse_pos_y) * 0.2);
        mouse_pos_x = new_x;
        mouse_pos_y = new_y;
        return true;
        
    }
    return false;


}
void BasicScene::renderOverlay()
{
    rtps::render::Text text;
    float start_pos_x = -0.96;
    float start_pos_y = 0.92;
    int w = m_window->width();
    int h = m_window->height();
    float4 color(1, 1, 0, 0);
    if(show_info) {
        //text.draw("FPS", start_pos_x, start_pos_y, w, h, color);
        renderKeyValue("Maximum Number of Particles", ps->settings->GetSettingAs<int>("Maximum Number of Particles"), start_pos_x, start_pos_y, color, text);
        renderKeyValue("Number of Particles", ps->settings->GetSettingAs<int>("Number of Particles"), start_pos_x, start_pos_y - 0.03, color, text);
        renderKeyValue("Mass", ps->settings->GetSettingAs<float>("Mass"), start_pos_x, start_pos_y - 0.06, color, text);
        renderKeyValue("Smoothing Distance (h)", ps->settings->GetSettingAs<float>("Smoothing Distance"), start_pos_x, start_pos_y - 0.09, color, text);
        renderKeyValue("Gas Constant (K)", ps->settings->GetSettingAs<float>("Gas Constant"), start_pos_x, start_pos_y - 0.12, color, text);
        renderKeyValue("Gravity", ps->settings->GetSettingAs<float>("Gravity"), start_pos_x, start_pos_y - 0.15, color, text);
        renderKeyValue("Viscosity", ps->settings->GetSettingAs<float>("Viscosity"), start_pos_x, start_pos_y - 0.18, color, text);
        renderKeyValue("Velocity Limit", ps->settings->GetSettingAs<float>("Velocity Limit"), start_pos_x, start_pos_y - 0.21, color, text);
    }

    //==================================================
    if(show_help) {
        start_pos_x = 0.65;
        start_pos_y = 0.92;
        text.draw("Help Information:", start_pos_x, start_pos_y, w, h, color);
        text.draw("H: Show Help Information", start_pos_x, start_pos_y - 0.03, w, h, color);
        text.draw("b: Add a bunny water", start_pos_x, start_pos_y - 0.06, w, h, color);
        text.draw("i: Show system information", start_pos_x, start_pos_y - 0.09, w, h, color);
        text.draw("p: change render method", start_pos_x, start_pos_y - 0.12, w, h, color);
        text.draw("Q/q/ESC: Quit", start_pos_x, start_pos_y - 0.15, w, h, color);
        text.draw("e: Add Dame Break(water)", start_pos_x, start_pos_y - 0.18, w, h, color);
        text.draw("o: Add Dame Break(oil)", start_pos_x, start_pos_y - 0.21, w, h, color);
        text.draw("t: Pause simulation", start_pos_x, start_pos_y - 0.24, w, h, color);
        text.draw("l: Push Fluid", start_pos_x, start_pos_y - 0.27, w, h, color);
        text.draw("9: Two hose", start_pos_x, start_pos_y - 0.30, w, h, color);
        text.draw("m: Two box", start_pos_x, start_pos_y - 0.33, w, h, color);
        text.draw("8: Aadd buny oil", start_pos_x, start_pos_y - 0.36, w, h, color);
    }

}
template <typename T>
void BasicScene::renderKeyValue(const std::string& key, const T&value, float x, float y, const float4 & color, rtps::render::Text& text)
{
    int w = m_window->width();
    int h = m_window->height();
    std::stringstream ss;
    ss << key << ": " << value;
    text.draw(ss.str(), x, y, w, h, color);
}

