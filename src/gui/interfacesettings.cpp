/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "interfacesettings.h"
#include "ui_interfacesettings.h"

#include "guisettings.h"
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <algorithm>
#include <repository.h>
#include <theme.h>

namespace Gui {

InterfaceSettings::InterfaceSettings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InterfaceSettings)
{
    ui->setupUi(this);

    ui->styleCombo->addItems({"[Default]", "Fusion", "Oreo"});

    // Help Path
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp(); // Move up one directory level
    dir.cd("site"); // Append "site" to the path
    QString path = QUrl::fromLocalFile(dir.absolutePath()).toString();
    ui->pathLineEdit->setText(path);

    connect(ui->openHelpPath, &QPushButton::clicked, this, &InterfaceSettings::browseHelpPath);

    KSyntaxHighlighting::Repository repository;
    const auto themes = repository.themes();
    QStringList themeNames = {"[Default]"};
    auto themeToString = [](const auto &theme) {
        return theme.name();
    };
    std::ranges::transform(themes, std::back_inserter(themeNames), themeToString);
    ui->themeCombo->addItems(themeNames);

    initialize();

    auto setStyle = [](int idx) {
        GuiSettings::instance()->setStyle(static_cast<GuiSettings::Style>(idx));
    };
    connect(ui->styleCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, setStyle);

    auto setTheme = [this](int idx) {
        GuiSettings::instance()->setTheme(idx == 0 ? "" : ui->themeCombo->currentText());
    };
    connect(ui->themeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, setTheme);

    auto setFontFamily = [](const QString &text) {
        GuiSettings::instance()->setFontFamily(text);
    };
    connect(ui->fontCombo, &QComboBox::currentTextChanged, this, setFontFamily);

    auto setFontSize = [](const QString &text) {
        GuiSettings::instance()->setFontSize(text.toInt());
    };
    connect(ui->fontSizeCombo, &QComboBox::currentTextChanged, this, setFontSize);

    auto setWordWrap = [](bool checked) {
        GuiSettings::instance()->setWordWrap(checked);
    };
    connect(ui->wordWrapCheck, &QCheckBox::toggled, this, setWordWrap);
}

InterfaceSettings::~InterfaceSettings() = default;

void InterfaceSettings::initialize()
{
    ui->styleCombo->setCurrentIndex(static_cast<int>(GuiSettings::instance()->style()));
    const int themeIdx = ui->themeCombo->findText(GuiSettings::instance()->theme());
    ui->themeCombo->setCurrentIndex(themeIdx == -1 ? 0 : themeIdx);

    ui->fontCombo->setCurrentText(GuiSettings::instance()->fontFamily());
    ui->fontSizeCombo->setCurrentText(QString::number(GuiSettings::instance()->fontSize()));
    ui->wordWrapCheck->setChecked(GuiSettings::instance()->isWordWrap());
}

QString InterfaceSettings::getHelpPath()
{
    return ui->pathLineEdit->text();
}

void InterfaceSettings::browseHelpPath()
{
    QString initialPath = getHelpPath();
    QString newPath = QFileDialog::getExistingDirectory(nullptr, tr("Select Help Path"), initialPath);
    if (!newPath.isEmpty()) {
        //  Update the line edit with the selected path
        ui->pathLineEdit->setText(newPath);
    }
}

} // namespace Gui
