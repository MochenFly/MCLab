#include "Mainwindow.h"
#include "ui_Mainwindow.h"

Mainwindow::Mainwindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainwindowClass())
{
    initialize();

    m_pRender2DWidget->loadPNGFile("../Data/WuKong.png");
}

Mainwindow::~Mainwindow()
{
    delete ui;
}

void Mainwindow::initialize()
{
    initializeUi();
    initializeConnections();
}

void Mainwindow::initializeUi()
{
    ui->setupUi(this);

    m_pRender2DWidget = new Render2DWidget(this);
    ui->hLayoutRender->addWidget(m_pRender2DWidget);
}

void Mainwindow::initializeConnections()
{
    connect(ui->btnNone, &QPushButton::clicked, this, [&]()
    {
        m_pRender2DWidget->setInteractorStyle(InteractorStyle_None);
    });

    connect(ui->btnImage, &QPushButton::clicked, this, [&]()
    {
        m_pRender2DWidget->setInteractorStyle(InteractorStyle_Image);
    });

    connect(ui->btnRectangle, &QPushButton::clicked, this, [&]()
    {
        m_pRender2DWidget->setInteractorStyle(Selection_2D_Rectangle);
    });

    connect(ui->btnEllipse, &QPushButton::clicked, this, [&]()
    {
        m_pRender2DWidget->setInteractorStyle(Selection_2D_Ellipse);
    });
}
