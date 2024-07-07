#include "MCVideoPlayTest.h"
#include "ui_MCVideoPlayTest.h"

MCVideoPlayTest::MCVideoPlayTest(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MCVideoPlayTest())
{
    ui->setupUi(this);

    m_pVideoWidget = new MCWidget::MCVideoWidget(ui->wgtVideo);
    ui->hLayoutVideo->addWidget(m_pVideoWidget);

    m_pVideoPlayer = new MCWidget::MCVideoPlayer(this);
    connect(ui->btnPlay, &QPushButton::clicked, this, [&]()
    {
        m_pVideoPlayer->setVideoFilePath(QString::fromLocal8Bit("D:/Resource/Video/testVideo.avi"));
        m_pVideoPlayer->playVideo();
    });

    connect(m_pVideoPlayer, &MCWidget::MCVideoPlayer::sigFrameChanged, this, [&](std::shared_ptr<MCWidget::MCVideoFrame> frame)
    {
        m_pVideoWidget->updateFrame(frame);
    }, Qt::BlockingQueuedConnection);
}

MCVideoPlayTest::~MCVideoPlayTest()
{
    delete ui;
}
