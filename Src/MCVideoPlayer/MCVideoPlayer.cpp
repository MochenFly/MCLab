#include "MCVideoPlayer.h"
#include "ui_MCVideoPlayer.h"

MCVideoPlayer::MCVideoPlayer(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MCVideoPlayerClass())
{
    ui->setupUi(this);
}

MCVideoPlayer::~MCVideoPlayer()
{
    delete ui;
}
