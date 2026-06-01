#include "mainwindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    mainwindow w;  // 【这里改成全小写】
    w.show();
    return a.exec();
}