#include "MCVideoFrame.h"
USE_MCWIDGET_NAMESPACE

MCVideoFrame::MCVideoFrame(QObject* parent)
    : QObject(parent)
{}

MCVideoFrame::~MCVideoFrame()
{
    freeData();
}

void MCVideoFrame::setYUVData(const uint8_t* data, int width, int height)
{
    freeData();

    m_width = width;
    m_height = height;

    int dataSize = width * height * 3 / 2;
    m_YUVData = (uint8_t*)malloc(dataSize);
    memcpy(m_YUVData, data, dataSize);
}

uint8_t* MCVideoFrame::getYUVData()
{
    return m_YUVData;
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
}
