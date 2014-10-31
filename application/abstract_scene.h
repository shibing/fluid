#ifndef ABSTRACT_SCENE_H
#define ABSTRACT_SCENE_H

class QOpenGLContext;
class QKeyEvent; 

class OpenGLWindow;

class AbstractScene
{
public:
    AbstractScene() : m_context(0) {}
    virtual ~AbstractScene() {}

    void setContext(QOpenGLContext *context) { m_context = context; }
    QOpenGLContext* context() const { return m_context; }

    void setWindow(OpenGLWindow *window) { m_window = window; }
    OpenGLWindow* getWindow() const { return m_window; }

    virtual void initialize() = 0;
    virtual void update(float dt = 0)  = 0;
    virtual void render() = 0;
    virtual void resize(int width, int height) = 0;
    virtual bool keyPress(QKeyEvent * event) = 0 ;

protected:
    OpenGLWindow *m_window;
    QOpenGLContext *m_context;
};

#endif
