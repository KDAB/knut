#include "core/knutmain.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("knut");
    QApplication::setApplicationVersion(KNUT_VERSION);

    Q_INIT_RESOURCE(core);

    Core::KnutMain knut;
    knut.process(app);

    return app.exec();
}
