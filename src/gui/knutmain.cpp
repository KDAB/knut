#include "knutmain.h"

#include "mainwindow.h"

namespace Gui {

KnutMain::KnutMain(QObject *parent)
    : Core::KnutCore(parent)
{
}

void KnutMain::initParser(QCommandLineParser &parser) const
{
    Core::KnutCore::initParser(parser);
    parser.addOption({"ide", "Open Knut IDE"});
}

void KnutMain::doParse(const QCommandLineParser &parser) const
{
    bool ide = parser.isSet("ide");
    if (ide || parser.optionNames().isEmpty()) {
        auto ide = new MainWindow();
        ide->show();
    }
}

} // namespace Gui
