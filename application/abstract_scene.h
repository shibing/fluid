#ifndef ABSTRACT_SCENE_H
#define ABSTRACT_SCENE_H

class QOpenGLContext;

class AbstractScene
{
public:
    AbstractScene() : m_context(0) {}
    virtual ~AbstractScene() {}

    void setContext(QOpenGLContext *context) { m_context = context; }
    QOpenGLContext* context() const { return m_context; }

    virtual void initialize() = 0;
    virtual void update(float dt = 0)  = 0;
    virtual void render() = 0;
    virtual void resize(int width, int height) = 0;

protected:
    QOpenGLContext *m_context;
};

#endif
