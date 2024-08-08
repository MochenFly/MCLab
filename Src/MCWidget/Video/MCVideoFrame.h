#pragma once

#include "MCWidget_Global.h"
#include <QObject>

MCWIDGET_BEGIN_NAMESPACE

class MCWIDGET_EXPORT MCVideoFrame : public QObject
{
    Q_OBJECT

public:
    MCVideoFrame(QObject* parent = nullptr);
    ~MCVideoFrame();

    void setYUVData(const uint8_t* data, int width, int height);

    uint8_t* getYUVData();

    int getWidth();
    int getHeight();

private:
    void freeData();

private:
    uint8_t*        m_YUVData       { nullptr };

    int             m_width         { 0 };
    int             m_height        { 0 };
};

MCWIDGET_END_NAMESPACE
