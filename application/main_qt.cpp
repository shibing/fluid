#include "opengl_window.h"
#include "basic_scene.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>

#include <QtCore/qmath.h>

#include <cmath>
#include <RTPS.h>

using namespace rtps;

rtps::RTPS *ps;
#define NUM_PARTICLES 100000
#define DT 0.003f

static int window_width = 640 * 2;
static int window_height = 480 * 2;


int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    OpenGLWindow window(new BasicScene());
    window.show();
    //int max_num = rtps::nlpo2(NUM_PARTICLES);
    //rtps::Domain* grid = new Domain(float4(-2.5,-2.5,-2.5,0), float4(2.5, 2.5, 2.5, 0));
    //rtps::RTPSettings *settings = new rtps::RTPSettings(rtps::RTPSettings::SPH, NUM_PARTICLES, DT, grid);

    //settings->SetSetting("rtps_path", "./bin");

    //settings->setRenderType(RTPSettings::RENDER);
    //settings->setRadiusScale(0.4);
    //settings->setBlurScale(1.0);
    //settings->setUseGLSL(1);
    //settings->SetSetting("sub_intervals", 1);
    //settings->SetSetting("render_texture", "firejet_blast.png");
    //settings->SetSetting("render_frag_shader", "sprite_tex_frag.glsl");
    //settings->SetSetting("render_use_alpha", true);
    //settings->SetSetting("render_alpha_function", "add");
    //settings->SetSetting("lt_increment", -.00);
    //settings->SetSetting("lt_cl", "lifetime.cl");
    //ps = new rtps::RTPS(settings);

    return app.exec();
}

