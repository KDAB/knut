#include "knutmain.h"

#include "mainwindow.h"
#include "optionsdialog.h"
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
        {"gui", tr("Open Knut user interface")},
        {"gui-script", tr("Open the run script dialog")},
        {"gui-settings", tr("Open the settings dialog")},
    });
}

void KnutMain::doParse(const QCommandLineParser &parser) const
{
    bool ide = parser.isSet("gui");
    if (ide || parser.optionNames().isEmpty()) {
        auto ide = new MainWindow();
        ide->show();
        return;
    }

    bool runDialog = parser.isSet("gui-script");
    if (runDialog) {
        auto dialog = new RunScriptDialog;
        dialog->show();
        return;
    }

    bool settingsDialog = parser.isSet("gui-settings");
    if (settingsDialog) {
        auto dialog = new OptionsDialog;
        dialog->show();
        return;
    }
}

} // namespace Gui
