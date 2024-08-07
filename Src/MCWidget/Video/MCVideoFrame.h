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

    void setYUVData(const uint8_t* data, int width, int height, 
                    const uint8_t* y = nullptr,
                    const uint8_t* u = nullptr, 
                    const uint8_t* v = nullptr);

    uint8_t* getYUVData();

    uint8_t* getYData();
    uint8_t* getUData();
    uint8_t* getVData();

    int getWidth();
    int getHeight();

private:
    void freeData();

private:
    uint8_t*        m_YUVData       { nullptr };

    uint8_t*        m_YData         { nullptr };
    uint8_t*        m_UData         { nullptr };
    uint8_t*        m_VData         { nullptr };

    int             m_width         { 0 };
    int             m_height        { 0 };
};

MCWIDGET_END_NAMESPACE
