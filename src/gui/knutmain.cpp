#include "knutmain.h"

#include "mainwindow.h"
#include "runscriptdialog.h"
#include "settingsdialog.h"

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
        {"settings", "Open the settings dialog"},
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

    bool settingsDialog = parser.isSet("settings");
    if (settingsDialog) {
        auto dialog = new SettingsDialog;
        dialog->show();
        return;
    }
}

} // namespace Gui
