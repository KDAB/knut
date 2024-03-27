#include "gui/knutmain.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("KDAB");
    QApplication::setApplicationName("knut");
    QApplication::setApplicationVersion(KNUT_VERSION_STRING);
    QApplication::setWindowIcon(QIcon(":/gui/icons/knut-64.png"));

    Q_INIT_RESOURCE(core);
    Q_INIT_RESOURCE(rccore);
    Q_INIT_RESOURCE(gui);

    Gui::KnutMain knut;
    knut.process(app);

    return app.exec();
}
