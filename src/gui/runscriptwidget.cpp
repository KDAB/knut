/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "runscriptwidget.h"
#include "core/scriptmanager.h"
#include "ui_runscriptwidget.h"

#include <QApplication>
#include <QCompleter>
#include <QFileDialog>
#include <QPushButton>
#include <kdalgorithms.h>

namespace Gui {

RunScriptWidget::RunScriptWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RunScriptWidget)
{
    ui->setupUi(this);
    setWindowTitle(QApplication::applicationName() + ' ' + QApplication::applicationVersion() + " - " + windowTitle());

    setProperty("panelWidget", true);
    setFocusProxy(ui->lineEdit);

    connect(ui->openButton, &QToolButton::clicked, this, &RunScriptWidget::chooseScript);
    ui->runButton->setEnabled(false);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, [this](const QString &str) {
        ui->runButton->setEnabled(!str.trimmed().isEmpty());
    });
    connect(ui->runButton, &QToolButton::clicked, this, &RunScriptWidget::run);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &RunScriptWidget::run);
    connect(ui->closeButton, &QToolButton::clicked, this, &RunScriptWidget::close);

    const auto &list = Core::ScriptManager::instance()->scriptList();
    QStringList scriptNames;
    scriptNames.reserve(static_cast<int>(list.size()));
    for (const auto &item : list)
        scriptNames.push_back(item.name);
    auto completer = new QCompleter(scriptNames, this);
    ui->lineEdit->setCompleter(completer);
}

RunScriptWidget::~RunScriptWidget() = default;

void RunScriptWidget::run()
{
    if (ui->lineEdit->text().isEmpty())
        return;

    QString scriptName = ui->lineEdit->text();
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
}

void RunScriptWidget::open()
{
    show();
    ui->lineEdit->setFocus();
}

void RunScriptWidget::chooseScript()
{
    const QString fileName =
        QFileDialog::getOpenFileName(this, QString(), ui->lineEdit->text(), tr("Scripts (*.js *.qml)"));
    if (fileName.isEmpty())
        return;
    ui->lineEdit->setText(fileName);
}

} // namespace Gui
