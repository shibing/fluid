#ifndef OPENGL_WINDOW_H
#define OPENGL_WINDOW_H
#include <QtGui/QWindow>

class QOpenGLFunctions_4_3_Core;
class QOpenGLContext;
class AbstractScene;
class QMouseEvent;

class OpenGLWindow : public QWindow
{
    Q_OBJECT
public:
    explicit OpenGLWindow(AbstractScene *scene,int width = 640, int height = 480, QScreen *screen = 0);
    ~OpenGLWindow();

    QOpenGLFunctions_4_3_Core * getOpenGLFunctions() { return m_opengl_funcs; }

    int getWidth() { return m_width;}
    int getHeight() { return m_height;}

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

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
    int m_width, m_height;
    QOpenGLContext *m_context;
    QOpenGLFunctions_4_3_Core *m_opengl_funcs;
    AbstractScene *m_scene;
};

#endif
