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

    connect(ui->btnPlay, &QPushButton::clicked, this, [&]()
    {
        m_pVideoPlayer->setVideoFilePath(QString::fromLocal8Bit("D:/Resource/Video/testVideo.avi"));
        m_pVideoPlayer->playVideo();
    });
    connect(ui->btnStop, &QPushButton::clicked, this, [&](bool pause)
    {
        m_pVideoPlayer->pauseVideo(pause);
    });

    connect(m_pVideoPlayer, &MCWidget::MCVideoPlayer::sigFrameChanged, this, [&](std::shared_ptr<MCWidget::MCVideoFrame> frame)
    {
        m_pVideoWidget->updateFrame(frame);
    }, Qt::BlockingQueuedConnection);

    connect(m_pVideoPlayer, &MCWidget::MCVideoPlayer::sigTimeChanged, this, [&](qint64 time)
    {
        ui->sliderTime->blockSignals(true);
        ui->sliderTime->setValue(time);
        ui->sliderTime->blockSignals(false);
    });

    connect(ui->sliderTime, &QSlider::valueChanged, this, [&](int value)
    {
        m_pVideoPlayer->seek(value);
    });

    connect(m_pVideoPlayer, &MCWidget::MCVideoPlayer::sigStateChanged, this, [&](MCWidget::MCVideoPlayer::VideoState state)
    {
        if (MCWidget::MCVideoPlayer::VideoState::PlayingState == state)
        {
            m_pVideoPlayer->startTimer();
        }
    }, Qt::BlockingQueuedConnection);
}

MCVideoPlayTest::~MCVideoPlayTest()
{
    delete ui;
}
