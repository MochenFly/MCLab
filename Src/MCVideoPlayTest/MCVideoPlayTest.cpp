#include "MCVideoPlayTest.h"
#include "ui_MCVideoPlayTest.h"

MCVideoPlayTest::MCVideoPlayTest(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MCVideoPlayTest())
{
    ui->setupUi(this);
}

MCVideoPlayTest::~MCVideoPlayTest()
{
    delete ui;
}
