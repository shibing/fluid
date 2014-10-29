#ifndef OPENGL_WINDOW_H
#define OPENGL_WINDOW_H
#include <QtGui/QWindow>

class QOpenGLFunctions_4_3_Core;
class QOpenGLContext;
class AbstractScene;

class OpenGLWindow : public QWindow
{
    Q_OBJECT
public:
    explicit OpenGLWindow(AbstractScene *scene,int width = 640 * 2, int height = 480 * 2, QScreen *screen = 0);
    ~OpenGLWindow();

    QOpenGLFunctions_4_3_Core * getOpenGLFunctions() { return m_opengl_funcs; }

signals:

public slots:

protected slots:
    void resizeGL();
    void paintGL();
    void updateScene();

private:
    void printContextInfos();
    void initializeGL();

private:
    QOpenGLContext *m_context;
    QOpenGLFunctions_4_3_Core *m_opengl_funcs;
    AbstractScene *m_scene;
};

#endif
