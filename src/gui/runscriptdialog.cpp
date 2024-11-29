/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "runscriptdialog.h"
#include "core/scriptmanager.h"
#include "ui_runscriptdialog.h"

#include <QApplication>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <kdalgorithms.h>

namespace Gui {

constexpr char LastScript[] = "lastScriptRun";

RunScriptDialog::RunScriptDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RunScriptDialog)
{
    ui->setupUi(this);
    ui->comboBox->lineEdit()->setPlaceholderText("Name or path of a script");
    setWindowTitle(QApplication::applicationName() + ' ' + QApplication::applicationVersion() + " - " + windowTitle());

    connect(ui->toolButton, &QToolButton::clicked, this, &RunScriptDialog::chooseScript);
    auto okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(false);
    connect(ui->comboBox, &QComboBox::currentTextChanged, this, [okButton](const QString &str) {
        okButton->setEnabled(!str.trimmed().isEmpty());
    });

    const auto &list = Core::ScriptManager::instance()->scriptList();
    QStringList scriptNames;
    scriptNames.reserve(static_cast<int>(list.size()));
    for (const auto &item : list)
        scriptNames.push_back(item.name);
    ui->comboBox->addItems(scriptNames);

    // Load the latest script name from the settings
    QSettings settings;
    const auto &lastScript = settings.value(LastScript).toString();
    if (!lastScript.isEmpty())
        ui->comboBox->setCurrentText(lastScript);
}

RunScriptDialog::~RunScriptDialog() = default;

void RunScriptDialog::accept()
{
    if (ui->comboBox->currentText().isEmpty())
        return;

    QString scriptName = ui->comboBox->currentText();
    // Store the latest script run from the dialog
    QSettings settings;
    settings.setValue(LastScript, scriptName);

    QFileInfo fi(scriptName);
    if (!fi.exists()) {
        const auto &list = Core::ScriptManager::instance()->scriptList();
        auto result = kdalgorithms::find_if(list, [&scriptName](const auto &item) {
            return item.name == scriptName;
        });
        if (!result)
            return;
        scriptName = result->fileName;
    }

    Core::ScriptManager::instance()->runScript(scriptName);
    QDialog::accept();
}

void RunScriptDialog::chooseScript()
{
    const QString fileName =
        QFileDialog::getOpenFileName(this, QString(), ui->comboBox->currentText(), tr("Scripts (*.js *.qml)"));
    if (fileName.isEmpty())
        return;
    ui->comboBox->setCurrentText(fileName);
}

} // namespace Gui
