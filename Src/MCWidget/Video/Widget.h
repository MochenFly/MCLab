#ifndef WIDGET_H
#define WIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include "MCVideoFrame.h"
USE_MCWIDGET_NAMESPACE

MCWIDGET_BEGIN_NAMESPACE

class MCWIDGET_EXPORT Widget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    Widget(QWidget* parent = nullptr);
    ~Widget();
 
    void updateFrame(std::shared_ptr<MCWidget::MCVideoFrame> frame);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    QOpenGLShaderProgram *program;
    GLuint textureY, textureU, textureV;
 
    int                     m_videoWidth;           // 视频宽度
    int                     m_videoHeight;          // 视频高度

    std::shared_ptr<MCWidget::MCVideoFrame> m_pVideoFrame;
};

MCWIDGET_END_NAMESPACE

#endif // WIDGET_H
