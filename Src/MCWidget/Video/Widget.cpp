#include "widget.h"

Widget::Widget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_videoHeight = 0;
    m_videoWidth = 0;
}

Widget::~Widget()
{
    delete program;
}

void Widget::updateFrame(std::shared_ptr<MCWidget::MCVideoFrame> frame)
{
    int width = frame.get()->getWidth();
    int height = frame.get()->getHeight();
    if (width <= 0 || height <= 0)
    {
        return;
    }

    m_pVideoFrame = frame;

    if (m_videoWidth != width || m_videoHeight != height)
    {
        m_videoWidth = width;
        m_videoHeight = height;
    }

    update();
}

void Widget::initializeGL()
{
    initializeOpenGLFunctions();

    glGenTextures(1, &textureY);
    glBindTexture(GL_TEXTURE_2D, textureY);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &textureU);
    glBindTexture(GL_TEXTURE_2D, textureU);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &textureV);
    glBindTexture(GL_TEXTURE_2D, textureV);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    program = new QOpenGLShaderProgram(this);
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Video/Shaders/vertex_shader.glsl");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Video/Shaders/fragment_shader.glsl");

    program->link();
    program->bind();
}

void Widget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void Widget::paintGL()
{
    if (nullptr != m_pVideoFrame)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureY);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_videoWidth, m_videoHeight, 0, GL_RED, GL_UNSIGNED_BYTE, m_pVideoFrame->getYData());

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureU);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_videoWidth / 2, m_videoHeight / 2, 0, GL_RED, GL_UNSIGNED_BYTE, m_pVideoFrame->getUData());

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureV);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_videoWidth / 2, m_videoHeight / 2, 0, GL_RED, GL_UNSIGNED_BYTE, m_pVideoFrame->getVData());

        program->bind();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    //glClear(GL_COLOR_BUFFER_BIT);

    //{
    //    glBindTexture(GL_TEXTURE_2D, textureY);
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_videoWidth, m_videoHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pVideoFrame->getYData());

    //    glBindTexture(GL_TEXTURE_2D, textureU);
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_videoWidth / 2, m_videoHeight / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pVideoFrame->getUData());

    //    glBindTexture(GL_TEXTURE_2D, textureV);
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_videoWidth / 2, m_videoHeight / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pVideoFrame->getVData());

    //    program->setUniformValue("texY", 0);
    //    program->setUniformValue("texU", 1);
    //    program->setUniformValue("texV", 2);

    //    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    //}
}
