#include "MCVideoPlayer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MCVideoPlayer w;
    w.show();
    return a.exec();
}
