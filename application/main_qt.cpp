#include "opengl_window.h"
#include "basic_scene.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>

#include <QtCore/qmath.h>

#include <cmath>
#include <RTPS.h>
#include <cstdio>

using namespace rtps;

rtps::RTPS *ps;

int  win_width = 640 ;
int  win_height = 640 ;

FILE* ffmpeg;

int main(int argc, char **argv)
{
    const char *cmd = "ffmpeg -r 60 -f rawvideo -pix_fmt rgba -s 640x640 -i - -threads 0 -preset fast -y -crf 21 -vf vflip output.mp4";
    ffmpeg = popen(cmd, "we");

    QGuiApplication app(argc, argv);
    OpenGLWindow window(new BasicScene(), win_width, win_height);
    window.show();
    return app.exec();
}

