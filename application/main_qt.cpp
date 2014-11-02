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

static int window_width = 640 ;
static int window_height = 480 ;


int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    OpenGLWindow window(new BasicScene(), 1280, 960);
    window.show();
    return app.exec();
}

