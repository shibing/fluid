#include "opengl_window.h"
#include "abstract_scene.h"

#include <QtGui/QOpenGLContext>
#include <QTimer>

QOpenGLContext *context;

OpenGLWindow::OpenGLWindow(AbstractScene *scene, int width, int height, QScreen *screen):
    QWindow(screen),
    m_scene(scene)
{
    setSurfaceType(OpenGLSurface);
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setMajorVersion(4);
    format.setMinorVersion(3);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);
    
    setFormat(format);
    create();

    m_context = new QOpenGLContext();
    m_context->setFormat(format);
    m_context->create();
    
    m_scene->setContext(m_context);
    context = m_context;


    scene->setWindow(this);

    initializeGL();

    resize(QSize(width, height));

    connect(this, SIGNAL(widthChanged(int)), this, SLOT(resizeGL()));
    connect(this, SIGNAL(heightChanged(int)), this, SLOT(resizeGL()));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateScene()));

    timer->start(16);
}

OpenGLWindow::~OpenGLWindow()
{
}

void OpenGLWindow::initializeGL()
{
    m_context->makeCurrent(this);
    m_scene->initialize();
}

void OpenGLWindow::paintGL()
{
    if(!isExposed())
        return;
    m_context->makeCurrent(this);
    m_scene->render();
    m_context->swapBuffers(this);
}

void OpenGLWindow::resizeGL()
{
    m_context->makeCurrent(this);
    m_scene->resize(width(), height());
}

void OpenGLWindow::updateScene()
{
    m_scene->update(0);
    paintGL();
}

        






