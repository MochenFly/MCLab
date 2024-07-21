#include "MCVideoPlayTest.h"
#include "ui_MCVideoPlayTest.h"

MCVideoPlayTest::MCVideoPlayTest(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MCVideoPlayTest())
{
    ui->setupUi(this);

    m_pVideoWidget = new MCWidget::MCVideoWidget(ui->wgtVideo);
    ui->hLayoutVideo->addWidget(m_pVideoWidget);

    ui->sliderTime->setValueByCliucked(true);

    m_pVideoPlayer = new MCWidget::MCVideoPlayer(this);
    m_pVideoPlayer->setVideoFilePath(QString::fromLocal8Bit("D:/Resource/Video/testVideo.mp4"));

    m_pTimer = new QTimer(this);
    connect(m_pTimer, &QTimer::timeout, this, &MCVideoPlayTest::timerTimeOut);
    m_pTimer->setInterval(500);
    
    // 视频播放、停止
    connect(ui->btnPlayOrStop, &QPushButton::clicked, this, &MCVideoPlayTest::videoPlayOrStop);
    // 视频跳转
    connect(ui->sliderTime, &QSlider::valueChanged, this, &MCVideoPlayTest::videoSeek);

    // 视频总时长更新
    connect(m_pVideoPlayer, &MCWidget::MCVideoPlayer::sigDurationChanged, 
            this, &MCVideoPlayTest::videoDurationChanged);
    // 视频数据帧更新
    connect(m_pVideoPlayer, &MCWidget::MCVideoPlayer::sigFrameChanged, 
            this, &MCVideoPlayTest::videoFrameChanged);
    // 视频播放状态更新
    connect(m_pVideoPlayer, &MCWidget::MCVideoPlayer::sigStateChanged, 
            this, &MCVideoPlayTest::videoStateChanged);
}

MCVideoPlayTest::~MCVideoPlayTest()
{
    delete ui;
}

void MCVideoPlayTest::timerTimeOut()
{
    qint64 currentTime = m_pVideoPlayer->getCurrentTime();
    ui->sliderTime->blockSignals(true);
    ui->sliderTime->setValue(currentTime);
    ui->sliderTime->blockSignals(false);
    ui->labelCurrentTime->setText(getTimeString(currentTime));
}

void MCVideoPlayTest::videoPlayOrStop()
{
    MCWidget::MCVideoPlayer::VideoState state = m_pVideoPlayer->getState();
    if (MCWidget::MCVideoPlayer::VideoState::StoppedState == state)
    {
        m_pVideoPlayer->playVideo();
    }
    else
    {
        m_pVideoPlayer->stopVideo();
    }
}

void MCVideoPlayTest::videoSeek(qint64 seekTime)
{
    m_pVideoPlayer->seekVideo(seekTime);
    ui->labelCurrentTime->setText(getTimeString(seekTime));
}

void MCVideoPlayTest::videoDurationChanged(qint64 msecond)
{
    ui->sliderTime->setRange(0, msecond);
    ui->labelTotalTime->setText(getTimeString(msecond));
}

void MCVideoPlayTest::videoFrameChanged(std::shared_ptr<MCWidget::MCVideoFrame> frame)
{
    m_pVideoWidget->updateFrame(frame);
}

QString MCVideoPlayTest::getTimeString(qint64 msecond)
{
    qint64 hours = msecond / 3600000;
    qint64 mseconds = msecond % 3600000;
    qint64 minutes = mseconds / 60000;
    mseconds = mseconds % 60000;
    qint64 seconds = mseconds / 1000;
    return QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}

void MCVideoPlayTest::videoStateChanged(MCWidget::MCVideoPlayer::VideoState state)
{
    if (MCWidget::MCVideoPlayer::VideoState::PlayingState == state)
    {
        ui->btnPlayOrStop->setText("stop");
        m_pTimer->start();
    }
    else
    {
        ui->btnPlayOrStop->setText("play");
        m_pTimer->stop();
        timerTimeOut();
    }
}
