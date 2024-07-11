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
    void setVideoWidth(int width, int height);

    void resetGLVertex(int window_W, int window_H);

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

private:
    QOpenGLShader* m_pVertexShader;  //顶点着色器程序对象
    QOpenGLShader* m_pFragmentShader;  //片段着色器对象

    int m_videoWidth;
    int m_videoHeight;

    ///OPenGL用于绘制图像
    GLuint textureUniformY; //y纹理数据位置
    GLuint textureUniformU; //u纹理数据位置
    GLuint textureUniformV; //v纹理数据位置
    GLuint id_y; //y纹理对象ID
    GLuint id_u; //u纹理对象ID
    GLuint id_v; //v纹理对象ID
    QOpenGLTexture* m_pTextureY;  //y纹理对象
    QOpenGLTexture* m_pTextureU;  //u纹理对象
    QOpenGLTexture* m_pTextureV;  //v纹理对象
    QOpenGLShaderProgram* m_pShaderProgram; //着色器程序容器
    QOpenGLShaderProgram* m_program;
    GLfloat* m_vertexVertices; // 顶点矩阵

    float mPicIndex_X; //按比例显示情况下 图像偏移量百分比 (相对于窗口大小的)
    float mPicIndex_Y; //

    bool mIsOpenGLInited; //openGL初始化函数是否执行过了

    ///OpenGL用于绘制矩形
    GLuint m_posAttr;
    GLuint m_colAttr;

    std::shared_ptr<MCWidget::MCVideoFrame> m_pVideoFrame;
};

MCWIDGET_END_NAMESPACE
