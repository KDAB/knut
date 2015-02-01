#include <QApplication>
#include "TutorialDlg.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Create and show our dialog
    CTutorialDlg dlg;
    dlg.exec();

    return app.exec();
}
