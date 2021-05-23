#include "interface/knutmain.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("knut");
    QApplication::setApplicationVersion(KNUT_VERSION);

    Q_INIT_RESOURCE(core);
    Q_INIT_RESOURCE(interface);

    Interface::KnutMain knut;
    knut.process(app);

    return app.exec();
}
