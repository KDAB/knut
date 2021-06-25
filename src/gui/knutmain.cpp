#include "knutmain.h"

#include "mainwindow.h"
#include "runscriptdialog.h"

namespace Gui {

KnutMain::KnutMain(QObject *parent)
    : Core::KnutCore(parent)
{
}

void KnutMain::initParser(QCommandLineParser &parser) const
{
    Core::KnutCore::initParser(parser);
    parser.addOptions({
        {"ide", "Open Knut IDE"},
        {"run-dialog", "Open the run script dialog"},
    });
}

void KnutMain::doParse(const QCommandLineParser &parser) const
{
    bool ide = parser.isSet("ide");
    if (ide || parser.optionNames().isEmpty()) {
        auto ide = new MainWindow();
        ide->show();
        return;
    }

    bool runDialog = parser.isSet("run-dialog");
    if (runDialog) {
        auto dialog = new RunScriptDialog;
        dialog->show();
        return;
    }
}

} // namespace Gui
