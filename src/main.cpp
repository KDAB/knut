#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("KDAB"));
    QCoreApplication::setApplicationName(QStringLiteral("Knut"));
    QCoreApplication::setApplicationVersion(QStringLiteral("3.0"));

    MainWindow w;
    w.show();

    return a.exec();
}
