#pragma once

#include <QtWidgets/QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MCVideoPlayerClass; };
QT_END_NAMESPACE

class MCVideoPlayer : public QMainWindow
{
    Q_OBJECT

public:
    MCVideoPlayer(QWidget* parent = nullptr);
    ~MCVideoPlayer();

private:
    Ui::MCVideoPlayerClass* ui;
};
