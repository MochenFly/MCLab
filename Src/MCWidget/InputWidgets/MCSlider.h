#pragma once

#include <QSlider>
#include "MCWidget_Global.h"

MCWIDGET_BEGIN_NAMESPACE

class MCWIDGET_EXPORT MCSlider : public QSlider
{
    Q_OBJECT

public:
    MCSlider(QWidget* parent);
    MCSlider(Qt::Orientation orientation, QWidget* parent = nullptr);
    ~MCSlider();

    void setValueByCliucked(bool valueByCliucked);

protected:
    void mousePressEvent(QMouseEvent* event);

private:
    bool        m_valueByCliucked       { false };
};

MCWIDGET_END_NAMESPACE
