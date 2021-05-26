#include "ui/knutmain.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("knut");
    QApplication::setApplicationVersion(KNUT_VERSION);

    Q_INIT_RESOURCE(core);

    Ui::KnutMain knut;
    knut.process(app);

    return app.exec();
}
