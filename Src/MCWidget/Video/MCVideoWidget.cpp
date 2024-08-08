#include "MCVideoWidget.h"
USE_MCWIDGET_NAMESPACE

// 顶点矩阵
static const GLfloat s_vertexVertices[] =
{
   -1.0f, -1.0f,
    1.0f, -1.0f,
   -1.0f,  1.0f,
    1.0f,  1.0f,
};

// 纹理矩阵
static const GLfloat s_textureVertices[] =
{
    0.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
};

MCVideoWidget::MCVideoWidget(QWidget* parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f)
{
    m_pVertexShader = nullptr;
    m_pFragmentShader = nullptr;
    m_pShaderProgram = nullptr;

    m_pVertexVertices = new GLfloat[8];

    m_textureIdY = 0;
    m_textureIdU = 0;
    m_textureIdV = 0;

    m_textureUniformY = 0;
    m_textureUniformU = 0;
    m_textureUniformV = 0;

    m_videoHeight = 0;
    m_videoWidth = 0;

    m_pVideoFrame = nullptr;
}

MCVideoWidget::~MCVideoWidget()
{
    // 确保当前 OpenGL 上下文是有效的
    makeCurrent();

    delete m_pShaderProgram;

    delete[] m_pVertexVertices;

    // 清理完成后释放当前上下文
    doneCurrent();

    m_pVideoFrame = nullptr;
}

void MCVideoWidget::updateFrame(std::shared_ptr<MCWidget::MCVideoFrame> frame)
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

        updateGLVertex(this->width(), this->height());
    }

    update();
}

void MCVideoWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);

    // 顶点着色器
    m_pVertexShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char* vsrc =
        "attribute vec4 vertexIn;           \n"
        "attribute vec2 textureIn;          \n"
        "varying vec2 textureOut;           \n"
        "void main(void)                    \n"
        "{                                  \n"
        "   gl_Position = vertexIn;         \n"
        "   textureOut = textureIn;         \n"
        "}";
    m_pVertexShader->compileSourceCode(vsrc);

    // 片段着色器
    m_pFragmentShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char* fsrc =
        "#ifdef GL_ES                                           \n"
        "precision mediump float;                               \n"
        "#endif                                                 \n"
        "varying vec2 textureOut;                               \n"
        "uniform sampler2D tex_y;                               \n"
        "uniform sampler2D tex_u;                               \n"
        "uniform sampler2D tex_v;                               \n"
        "void main(void)                                        \n"
        "{                                                      \n"
        "   vec3 yuv;                                           \n"
        "   vec3 rgb;                                           \n"
        "   yuv.x = texture2D(tex_y, textureOut).r;             \n"
        "   yuv.y = texture2D(tex_u, textureOut).r - 0.5;       \n"
        "   yuv.z = texture2D(tex_v, textureOut).r - 0.5;       \n"
        "   rgb = mat3( 1,      1,         1,                   \
                        0,      -0.187324, 1.8556,              \
                        1.5748, -0.468124, 0 ) * yuv;           \n"
        "   gl_FragColor = vec4(rgb, 1);                        \n"
        "}";
    m_pFragmentShader->compileSourceCode(fsrc);

    // 创建着色器程序容器
    m_pShaderProgram = new QOpenGLShaderProgram();
    // 将顶点着色器添加到程序容器
    m_pShaderProgram->addShader(m_pVertexShader);
    // 将片段着色器添加到程序容器
    m_pShaderProgram->addShader(m_pFragmentShader);
    // 绑定顶点属性 
    m_pShaderProgram->bindAttributeLocation("vertexIn", 3);
    // 绑定纹理属性
    m_pShaderProgram->bindAttributeLocation("textureIn", 4);
    // 链接着色器程序
    m_pShaderProgram->link();
    // 激活所有链接
    m_pShaderProgram->bind();

    // 读取着色器中的数据变量 tex_y, tex_u, tex_v 的位置
    m_textureUniformY = m_pShaderProgram->uniformLocation("tex_y");
    m_textureUniformU = m_pShaderProgram->uniformLocation("tex_u");
    m_textureUniformV = m_pShaderProgram->uniformLocation("tex_v");

    // 生成纹理，获取 Y 纹理索引值
    glGenTextures(1, &m_textureIdY);
    // 生成纹理，获取 U 纹理索引值
	glGenTextures(1, &m_textureIdU);
    // 生成纹理，获取 V 纹理索引值
	glGenTextures(1, &m_textureIdV);

    // 设置 Y 纹理参数
    glBindTexture(GL_TEXTURE_2D, m_textureIdY);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 设置 U 纹理参数
    glBindTexture(GL_TEXTURE_2D, m_textureIdU);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 设置 V 纹理参数
    glBindTexture(GL_TEXTURE_2D, m_textureIdV);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 设置读取的 YUV 数据为 1 字节对齐，
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 设置清除颜色
    glClearColor(0.0, 0.0, 0.0, 0.0);

    updateGLVertex(this->width(), this->height());
}

void MCVideoWidget::updateGLVertex(int windowWidth, int widowHeight)
{
    if (m_videoWidth <= 0 || m_videoHeight <= 0)
    {
        memcpy(m_pVertexVertices, s_vertexVertices, sizeof(s_vertexVertices));

        // 设置顶点矩阵值以及格式
        glVertexAttribPointer(3, 2, GL_FLOAT, 0, 0, m_pVertexVertices);
        // 设置纹理矩阵值以及格式
        glVertexAttribPointer(4, 2, GL_FLOAT, 0, 0, s_textureVertices);
        // 启用顶点属性
        glEnableVertexAttribArray(3);
        // 启用纹理属性
        glEnableVertexAttribArray(4);
    }
    else
    {
        // 以宽度为基准缩放视频
        int width = windowWidth;
        int height = m_videoHeight * width / m_videoWidth;
        int x = this->width() - width;
        int y = this->height() - height;
        x /= 2;
        y /= 2;

        // 显示不全时则以高度为基准缩放视频
        if (y < 0)
        {
            height = widowHeight;
            width = m_videoWidth * height / m_videoHeight;
            x = this->width() - width;
            y = this->height() - height;
            x /= 2;
            y /= 2;
        }

        float index_x = x * 1.0 / windowWidth * 2.0 - 1.0;
        float index_x_1 = index_x * -1.0;
        float index_x_2 = index_x;

        float index_y = y * 1.0 / widowHeight * 2.0 - 1.0;
        float index_y_1 = index_y * -1.0;
        float index_y_2 = index_y;

        const GLfloat vertexVertices[] =
        {
            index_x_2, index_y_2,
            index_x_1, index_y_2,
            index_x_2, index_y_1,
            index_x_1, index_y_1,
        };
        memcpy(m_pVertexVertices, vertexVertices, sizeof(vertexVertices));

        // 设置顶点矩阵值以及格式
        glVertexAttribPointer(3, 2, GL_FLOAT, 0, 0, m_pVertexVertices);
        // 设置纹理矩阵值以及格式
        glVertexAttribPointer(4, 2, GL_FLOAT, 0, 0, s_textureVertices);
        // 启用顶点属性
        glEnableVertexAttribArray(3);
        // 启用纹理属性
        glEnableVertexAttribArray(4);
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

    updateGLVertex(width, height);
}

void MCVideoWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (nullptr != m_pVideoFrame.get())
    {
        uint8_t* pYUVData = m_pVideoFrame.get()->getYUVData();

        if (nullptr != pYUVData)
        {
            m_pShaderProgram->bind();

            // 加载 Y 数据纹理
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_textureIdY);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_videoWidth, m_videoHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pYUVData);

            // 加载 U 数据纹理
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_textureIdU);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_videoWidth / 2, m_videoHeight / 2, 0, GL_RED, GL_UNSIGNED_BYTE, 
                         (char*)pYUVData + m_videoWidth * m_videoHeight);
            
            // 加载 V 数据纹理
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, m_textureIdV);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_videoWidth / 2, m_videoHeight / 2, 0, GL_RED, GL_UNSIGNED_BYTE, 
                         (char*)pYUVData + m_videoWidth * m_videoHeight * 5 / 4);

            // 指定 Y 纹理要使用新值
            glUniform1i(m_textureUniformY, 0);
            // 指定 U 纹理要使用新值
            glUniform1i(m_textureUniformU, 1);
            // 指定 V 纹理要使用新值
            glUniform1i(m_textureUniformV, 2);

            // 使用顶点数组方式绘制图形
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            m_pShaderProgram->release();
        }
    }
}
