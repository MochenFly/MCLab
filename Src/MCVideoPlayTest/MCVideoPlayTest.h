#pragma once

#include <QtWidgets/QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MCVideoPlayTest; };
QT_END_NAMESPACE

class MCVideoPlayTest : public QMainWindow
{
    Q_OBJECT

public:
    MCVideoPlayTest(QWidget* parent = nullptr);
    ~MCVideoPlayTest();

private:
    Ui::MCVideoPlayTest* ui;
};
