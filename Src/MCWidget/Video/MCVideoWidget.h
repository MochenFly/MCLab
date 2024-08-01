#pragma once

#include "MCWidget_Global.h"
#include "MCVideoFrame.h"

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>

MCWIDGET_BEGIN_NAMESPACE

class MCWIDGET_EXPORT MCVideoWidget : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit MCVideoWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~MCVideoWidget();

    void updateFrame(std::shared_ptr<MCWidget::MCVideoFrame> frame);

private:
    void updateGLVertex(int windowWidth, int widowHeight);

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

private:
    QOpenGLShader*          m_pVertexShader;        // 顶点着色器
    QOpenGLShader*          m_pFragmentShader;      // 片段着色器
    QOpenGLShaderProgram*   m_pShaderProgram;       // 着色器程序

    QOpenGLTexture*         m_pTextureY;            // Y 纹理对象
    QOpenGLTexture*         m_pTextureU;            // U 纹理对象
    QOpenGLTexture*         m_pTextureV;            // V 纹理对象

    GLuint                  m_textureIdY;           // Y 纹理对象 id
    GLuint                  m_textureIdU;           // U 纹理对象 id
    GLuint                  m_textureIdV;           // V 纹理对象 id

    GLuint                  m_textureUniformY;      // Y 纹理位置
    GLuint                  m_textureUniformU;      // U 纹理位置
    GLuint                  m_textureUniformV;      // V 纹理位置

    int                     m_videoWidth;           // 视频宽度
    int                     m_videoHeight;          // 视频高度

    std::shared_ptr<MCWidget::MCVideoFrame> m_pVideoFrame;
};

MCWIDGET_END_NAMESPACE
