#include "MCVideoFrame.h"
USE_MCWIDGET_NAMESPACE

MCVideoFrame::MCVideoFrame(QObject* parent)
    : QObject(parent)
{}

MCVideoFrame::~MCVideoFrame()
{
    freeData();
}

void MCVideoFrame::setYUVData(const uint8_t* data, int width, int height, const uint8_t* y, const uint8_t* u, const uint8_t* v)
{
    freeData();

    m_width = width;
    m_height = height;

    int dataSize = width * height * 3 / 2;
    m_YUVData = (uint8_t*)malloc(dataSize);
    memcpy(m_YUVData, data, dataSize);

    int ySize = width * height;
    int uvSize = (width / 2) * (height / 2);

    m_YData = (uint8_t*)malloc(ySize);
    memcpy(m_YData, y, ySize);

    m_UData = (uint8_t*)malloc(uvSize);
    memcpy(m_UData, u, uvSize);

    m_VData = (uint8_t*)malloc(uvSize);
    memcpy(m_VData, v, uvSize);
}

uint8_t* MCVideoFrame::getYUVData()
{
    return m_YUVData;
}

uint8_t* MCVideoFrame::getYData()
{
    return m_YData;
}

uint8_t* MCVideoFrame::getUData()
{
    return m_UData;
}

uint8_t* MCVideoFrame::getVData()
{
    return m_VData;
}

int MCVideoFrame::getWidth()
{
    return m_width;
}

int MCVideoFrame::getHeight()
{
    return m_height;
}

void MCVideoFrame::freeData()
{
    if (nullptr != m_YUVData)
    {
        free(m_YUVData);
        m_YUVData = nullptr;
    }

    //if (nullptr != m_YData)
    //{
    //    free(m_YData);
    //    m_YData = nullptr;
    //}

    //if (nullptr != m_UData)
    //{
    //    free(m_UData);
    //    m_UData = nullptr;
    //}

    //if (nullptr != m_VData)
    //{
    //    free(m_VData);
    //    m_VData = nullptr;
    //}
}
