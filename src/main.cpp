#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("KDAB"));
    QCoreApplication::setApplicationName(QStringLiteral("Knut"));
    QCoreApplication::setApplicationVersion(QStringLiteral("3.0"));

    auto create = [] {
        auto mw = new MainWindow;
        mw->setAttribute(Qt::WA_DeleteOnClose);
        mw->show();
        return mw;
    };

    if (argc > 1) {
        // open one main window per file
        for (int i = 1; i < argc; ++i)
            create()->openFile(QFile::decodeName(argv[i]));
    } else {
        create();
    }

    return a.exec();
}
