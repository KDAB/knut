#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("knut"));
    QCoreApplication::setApplicationVersion(QStringLiteral("2.0"));

    MainWindow w;
    w.show();

    return a.exec();
}
