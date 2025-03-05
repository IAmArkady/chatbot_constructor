#include "mainwindow.h"
#include "vkauthwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VkAuthWindow w;
    w.show();
    return a.exec();
}
