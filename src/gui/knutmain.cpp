/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "knutmain.h"
#include "mainwindow.h"
#include "optionsdialog.h"
#include "runscriptdialog.h"

namespace Gui {

KnutMain::KnutMain(QObject *parent)
    : Core::KnutCore({}, parent)
{
}

void KnutMain::initParser(QCommandLineParser &parser) const
{
    Core::KnutCore::initParser(parser);
    parser.addOptions({
        {"gui-run", tr("Opens the run script dialog")},
        {"gui-settings", tr("Opens the settings dialog")},
    });
}

void KnutMain::doParse(const QCommandLineParser &parser) const
{
    const bool runDialog = parser.isSet("gui-run");
    if (runDialog) {
        auto dialog = new RunScriptDialog;
        dialog->show();
        return;
    }

    const bool settingsDialog = parser.isSet("gui-settings");
    if (settingsDialog) {
        auto dialog = new OptionsDialog;
        dialog->show();
        return;
    }

    // Default case: open the main window
    auto ide = new MainWindow();
    ide->show();
}

} // namespace Gui
