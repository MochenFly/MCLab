#include "MCVideoWidget.h"
USE_MCWIDGET_NAMESPACE

#include <QPainter>
#include <QDebug>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QFile>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QMouseEvent>

#include <QTimer>
#include <QDrag>
#include <QMimeData>

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QDateTime>

#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4

static const char* vertexShaderSource =
"attribute highp vec4 posAttr;\n"
"attribute lowp vec4 colAttr;\n"
"varying lowp vec4 col;\n"
"uniform highp mat4 matrix;\n"
"void main() {\n"
"   col = colAttr;\n"
"   gl_Position = posAttr;\n"
"}\n";

static const char* fragmentShaderSource =
"varying lowp vec4 col;\n"
"void main() {\n"
"   gl_FragColor = col;\n"
"}\n";

MCVideoWidget::MCVideoWidget(QWidget* parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f)
{
    textureUniformY = 0;
    textureUniformU = 0;
    textureUniformV = 0;
    id_y = 0;
    id_u = 0;
    id_v = 0;

    m_pVertexShader = NULL;
    m_pFragmentShader = NULL;
    m_pShaderProgram = NULL;
    m_pTextureY = NULL;
    m_pTextureU = NULL;
    m_pTextureV = NULL;

    m_vertexVertices = new GLfloat[8];

    m_videoHeight = 0;
    m_videoWidth = 0;

    mPicIndex_X = 0;
    mPicIndex_Y = 0;

    mIsOpenGLInited = false;
}

MCVideoWidget::~MCVideoWidget()
{
}

void MCVideoWidget::updateFrame(std::shared_ptr<MCWidget::MCVideoFrame> frame)
{
    m_pVideoFrame = frame;
    int width = frame.get()->getWidth();
    int height = frame.get()->getHeight();

    if (m_videoWidth <= 0 || m_videoHeight <= 0 || m_videoWidth != width || m_videoHeight != height)
    {
        setVideoWidth(width, height);
    }

    update();
}

void MCVideoWidget::setVideoWidth(int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }

    m_videoWidth = width;
    m_videoHeight = height;

    resetGLVertex(this->width(), this->height());
}

void MCVideoWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    // 顶点着色器
    m_pVertexShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char* vsrc = 
        "attribute vec4 vertexIn;       \n"
        "attribute vec2 textureIn;      \n"
        "varying vec2 textureOut;       \n"
        "void main(void)                \n"
        "{                              \n"
            "gl_Position = vertexIn;    \n"
            "textureOut = textureIn;    \n"
        "}";
    m_pVertexShader->compileSourceCode(vsrc);

    // 片段着色器
    m_pFragmentShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char* fsrc =
        "#ifdef GL_ES               \n"
        "precision mediump float;   \n"
        "#endif                     \n"
        "varying vec2 textureOut;   \n"
        "uniform sampler2D tex_y;   \n"
        "uniform sampler2D tex_u;   \n"
        "uniform sampler2D tex_v;   \n"
        "void main(void)            \n"
        "{                          \n"
            "vec3 yuv;              \n"
            "vec3 rgb;              \n"
            "yuv.x = texture2D(tex_y, textureOut).r;            \n"
            "yuv.y = texture2D(tex_u, textureOut).r - 0.5;      \n"
            "yuv.z = texture2D(tex_v, textureOut).r - 0.5;      \n"
            "rgb = mat3( 1, 1, 1, 0, -0.39465, 2.03211, 1.13983, -0.58060, 0) * yuv;        \n"
            "gl_FragColor = vec4(rgb, 1);       \n"
        "}";
    m_pFragmentShader->compileSourceCode(fsrc);

    // 用于绘制矩形
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_posAttr = m_program->attributeLocation("posAttr");
    m_colAttr = m_program->attributeLocation("colAttr");

    // 创建着色器程序容器
    m_pShaderProgram = new QOpenGLShaderProgram;
    // 将片段着色器添加到程序容器
    m_pShaderProgram->addShader(m_pFragmentShader);
    // 将顶点着色器添加到程序容器
    m_pShaderProgram->addShader(m_pVertexShader);
    // 绑定属性 vertexIn 到指定位置 ATTRIB_VERTEX
    m_pShaderProgram->bindAttributeLocation("vertexIn", ATTRIB_VERTEX);
    // 绑定属性 textureIn 到指定位置 ATTRIB_TEXTURE
    m_pShaderProgram->bindAttributeLocation("textureIn", ATTRIB_TEXTURE);
    // 链接所有所有添入到的着色器程序
    m_pShaderProgram->link();
    // 激活所有链接
    m_pShaderProgram->bind();
    // 读取着色器中的数据变量 tex_y, tex_u, tex_v 的位置
    textureUniformY = m_pShaderProgram->uniformLocation("tex_y");
    textureUniformU = m_pShaderProgram->uniformLocation("tex_u");
    textureUniformV = m_pShaderProgram->uniformLocation("tex_v");

    // 顶点矩阵
    const GLfloat vertexVertices[] =
    {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         -1.0f, 1.0f,
         1.0f, 1.0f,
    };
    memcpy(m_vertexVertices, vertexVertices, sizeof(vertexVertices));

    // 纹理矩阵
    static const GLfloat textureVertices[] =
    {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };

    // 设置读取的 YUV 数据为 1 字节对齐，
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 设置属性 ATTRIB_VERTEX 的顶点矩阵值以及格式
    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, m_vertexVertices);
    // 设置属性 ATTRIB_TEXTURE 的纹理矩阵值以及格式
    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, textureVertices);
    // 启用 ATTRIB_VERTEX 属性的数据,默认是关闭的
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    // 启用 ATTRIB_TEXTURE 属性的数据,默认是关闭的
    glEnableVertexAttribArray(ATTRIB_TEXTURE);
    // 分别创建 Y, U, V 纹理对象
    m_pTextureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureY->create();
    m_pTextureU->create();
    m_pTextureV->create();

    // 获取返回 Y 分量的纹理索引值
    id_y = m_pTextureY->textureId();
    // 获取返回 U 分量的纹理索引值
    id_u = m_pTextureU->textureId();
    // 获取返回 V 分量的纹理索引值
    id_v = m_pTextureV->textureId();
    glClearColor(0.0, 0.0, 0.0, 0.0);
}

void MCVideoWidget::resetGLVertex(int window_W, int window_H)
{
    if (m_videoWidth <= 0 || m_videoHeight <= 0)
    {
        mPicIndex_X = 0.0;
        mPicIndex_Y = 0.0;

        // 顶点矩阵
        const GLfloat vertexVertices[] = {
            -1.0f, -1.0f,
             1.0f, -1.0f,
             -1.0f, 1.0f,
             1.0f, 1.0f,
        };

        memcpy(m_vertexVertices, vertexVertices, sizeof(vertexVertices));

        // 纹理矩阵
        static const GLfloat textureVertices[] = {
            0.0f,  1.0f,
            1.0f,  1.0f,
            0.0f,  0.0f,
            1.0f,  0.0f,
        };
        // 设置属性 ATTRIB_VERTEX 的顶点矩阵值以及格式
        glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, m_vertexVertices);
        // 设置属性 ATTRIB_TEXTURE 的纹理矩阵值以及格式
        glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, textureVertices);
        // 启用 ATTRIB_VERTEX 属性的数据,默认是关闭的
        glEnableVertexAttribArray(ATTRIB_VERTEX);
        // 启用 ATTRIB_TEXTURE 属性的数据,默认是关闭的
        glEnableVertexAttribArray(ATTRIB_TEXTURE);
    }
    else
    {
        // 按比例
        int pix_W = window_W;
        int pix_H = m_videoHeight * pix_W / m_videoWidth;

        int x = this->width() - pix_W;
        int y = this->height() - pix_H;
        x /= 2;
        y /= 2;

        if (y < 0)
        {
            pix_H = window_H;
            pix_W = m_videoWidth * pix_H / m_videoHeight;

            x = this->width() - pix_W;
            y = this->height() - pix_H;
            x /= 2;
            y /= 2;

        }

        mPicIndex_X = x * 1.0 / window_W;
        mPicIndex_Y = y * 1.0 / window_H;

        float index_y = y * 1.0 / window_H * 2.0 - 1.0;
        float index_y_1 = index_y * -1.0;
        float index_y_2 = index_y;

        float index_x = x * 1.0 / window_W * 2.0 - 1.0;
        float index_x_1 = index_x * -1.0;
        float index_x_2 = index_x;

        const GLfloat vertexVertices[] =
        {
            index_x_2, index_y_2,
            index_x_1,  index_y_2,
            index_x_2, index_y_1,
            index_x_1,  index_y_1,
        };
        memcpy(m_vertexVertices, vertexVertices, sizeof(vertexVertices));

        static const GLfloat textureVertices[] =
        {
            0.0f,  1.0f,
            1.0f,  1.0f,
            0.0f,  0.0f,
            1.0f,  0.0f,
        };

        // 设置属性 ATTRIB_VERTEX
        glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, m_vertexVertices);
        // 设置属性 ATTRIB_TEXTURE
        glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, textureVertices);
        // 启用 ATTRIB_VERTEX 属性
        glEnableVertexAttribArray(ATTRIB_VERTEX);
        // 启用 ATTRIB_TEXTURE 属性
        glEnableVertexAttribArray(ATTRIB_TEXTURE);
    }
}

void MCVideoWidget::resizeGL(int width, int height)
{
    if (height == 0)
    {
        height = 1;
    }

    // 设置视口
    glViewport(0, 0, width, height);

    resetGLVertex(width, height);
}

void MCVideoWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (nullptr != m_pVideoFrame.get())
    {
        uint8_t* m_pBufYuv420p = m_pVideoFrame.get()->getYUVData();

        if (m_pBufYuv420p != NULL)
        {
            m_pShaderProgram->bind();

            // 加载 Y 数据纹理
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, id_y);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_videoWidth, m_videoHeight, 0, GL_RED, GL_UNSIGNED_BYTE, m_pBufYuv420p);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // 加载 U 数据纹理
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, id_u);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_videoWidth / 2, m_videoHeight / 2, 0, GL_RED, GL_UNSIGNED_BYTE, 
                         (char*)m_pBufYuv420p + m_videoWidth * m_videoHeight);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            // 加载 V 数据纹理
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, id_v);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_videoWidth / 2, m_videoHeight / 2, 0, GL_RED, GL_UNSIGNED_BYTE, 
                         (char*)m_pBufYuv420p + m_videoWidth * m_videoHeight * 5 / 4);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // 指定 Y 纹理要使用新值
            glUniform1i(textureUniformY, 0);
            // 指定 U 纹理要使用新值
            glUniform1i(textureUniformU, 1);
            // 指定 V 纹理要使用新值
            glUniform1i(textureUniformV, 2);
            // 使用顶点数组方式绘制图形
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            m_pShaderProgram->release();
        }
    }
}
