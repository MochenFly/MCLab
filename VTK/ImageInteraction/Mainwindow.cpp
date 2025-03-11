#include "Mainwindow.h"
#include "ui_Mainwindow.h"

Mainwindow::Mainwindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainwindowClass())
{
    initialize();
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

    connect(ui->btnSpline, &QPushButton::clicked, this, [&]()
    {
        m_pRender2DWidget->setInteractorStyle(InteractorStyle_ParametricSpline);
    });

    connect(ui->btnImageData, &QPushButton::clicked, this, [&]()
    {
        m_pRender2DWidget->loadPNGFile();
    });

    connect(ui->btnSplineData, &QPushButton::clicked, this, [&]()
    {
        m_pRender2DWidget->drawSplineLine();
    });

    connect(ui->btnClear, &QPushButton::clicked, this, [&]()
    {
        m_pRender2DWidget->clearData();
    });
}
