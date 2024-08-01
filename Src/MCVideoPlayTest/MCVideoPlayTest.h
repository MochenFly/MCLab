#pragma once

#include <QtWidgets/QMainWindow>

#include "MCWidget/MCVideoPlayer.h"
#include "MCWidget/MCVideoWidget.h"
#include "MCWidget/MCVideoFrame.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MCVideoPlayTest; };
QT_END_NAMESPACE

class MCVideoPlayTest : public QMainWindow
{
    Q_OBJECT

public:
    MCVideoPlayTest(QWidget* parent = nullptr);
    ~MCVideoPlayTest();

private slots:
    void timerTimeOut();
    void videoPlayOrStop();
    void videoSeek(int seekTime);
    void videoDurationChanged(int msecond);
    void videoFrameChanged(std::shared_ptr<MCWidget::MCVideoFrame> frame);
    void videoStateChanged(MCWidget::MCVideoPlayer::VideoState state);

private:
    QString getTimeString(int msecond);

private:
    Ui::MCVideoPlayTest*        ui;

    MCWidget::MCVideoPlayer*    m_pVideoPlayer      { nullptr };
    MCWidget::MCVideoWidget*    m_pVideoWidget      { nullptr };
    QTimer*                     m_pTimer            { nullptr };
};
