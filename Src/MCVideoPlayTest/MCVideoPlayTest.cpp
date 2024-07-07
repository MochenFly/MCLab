#include "MCVideoPlayTest.h"
#include "ui_MCVideoPlayTest.h"

MCVideoPlayTest::MCVideoPlayTest(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MCVideoPlayTest())
{
    ui->setupUi(this);

    m_pVideoPlayer = new MCWidget::MCVideoPlayer(this);
    connect(ui->btnPlay, &QPushButton::clicked, this, [&]()
    {
        m_pVideoPlayer->setVideoFilePath(QString::fromLocal8Bit("D:/Resource/Video/»ÆÐ¡B.mp4"));
        m_pVideoPlayer->playVideo();
    });
}

MCVideoPlayTest::~MCVideoPlayTest()
{
    delete ui;
}
