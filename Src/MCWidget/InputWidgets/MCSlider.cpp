#include "MCSlider.h"
USE_MCWIDGET_NAMESPACE

#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionSlider>

MCSlider::MCSlider(QWidget* parent)
    : QSlider(parent)
{
    setMouseTracking(true);
}

MCSlider::MCSlider(Qt::Orientation orientation, QWidget* parent)
{
    setMouseTracking(true);
}

MCSlider::~MCSlider()
{
}

void MCSlider::setValueByCliucked(bool valueByCliucked)
{
    m_valueByCliucked = valueByCliucked;
}

void MCSlider::mousePressEvent(QMouseEvent* event)
{
    if (m_valueByCliucked)
    {
        QStyleOptionSlider opt;
        initStyleOption(&opt);

        QRect handleRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
        if (handleRect.contains(event->pos()))
        {
            QSlider::mousePressEvent(event);
        }
        else
        {
            int value = 0;
            if (Qt::Horizontal == orientation())
            {
                value = QStyle::sliderValueFromPosition(minimum(), maximum(), event->pos().x(), width());
                setValue(value);
            }
            else
            {
                value = QStyle::sliderValueFromPosition(minimum(), maximum(), event->pos().y(), height());
                setValue(value);
            }
            emit valueChanged(value);
        }
    }
    else
    {
        QSlider::mousePressEvent(event);
    }
}
