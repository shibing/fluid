#ifndef OPENGL_WINDOW_H
#define OPENGL_WINDOW_H
#include <QtGui/QWindow>

class QOpenGLContext;
class AbstractScene;

class OpenGLWindow : public QWindow
{
    Q_OBJECT
public:
    explicit OpenGLWindow(AbstractScene *scene,int width = 640 * 2, int height = 480 * 2, QScreen *screen = 0);
    ~OpenGLWindow();

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
    AbstractScene *m_scene;
};

#endif
