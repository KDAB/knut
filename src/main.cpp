#include "gui/knutmain.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("KDAB");
    QApplication::setApplicationName("knut");
    QApplication::setApplicationVersion(KNUT_VERSION);

    Q_INIT_RESOURCE(core);

    Gui::KnutMain knut;
    knut.process(app);

    return app.exec();
}
