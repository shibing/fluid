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
    return app.exec();
}

