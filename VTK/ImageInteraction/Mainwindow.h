#pragma once

#include <QtWidgets/QMainWindow>

#include "Render2DWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainwindowClass; };
QT_END_NAMESPACE

class Mainwindow : public QMainWindow
{
    Q_OBJECT

public:
    Mainwindow(QWidget* parent = nullptr);
    ~Mainwindow();

private:
    void initialize();
    void initializeUi();
    void initializeConnections();

private:
    Ui::MainwindowClass*        ui;
    Render2DWidget*             m_pRender2DWidget           { nullptr };
};
