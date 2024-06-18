/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "optionsdialog.h"
#include "core/cppdocument_p.h"
#include "core/rcdocument.h"
#include "core/scriptmanager.h"
#include "core/settings.h"
#include "core/textdocument_p.h"
#include "ui_optionsdialog.h"

#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QIntValidator>
#include <QUrl>
#include <algorithm>

namespace Gui {

OptionsDialog::OptionsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    setWindowTitle(QApplication::applicationName() + ' ' + QApplication::applicationVersion() + " - " + windowTitle());

    auto font = ui->title->font();
    font.setPointSize(font.pointSize() + 4);
    font.setBold(true);
    ui->title->setFont(font);

    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->listWidget, &QListWidget::currentItemChanged, this, &OptionsDialog::changePage);

    initializeScriptPathSettings();
    initializeScriptBehaviorSettings();
    initializeRcSettings();
    initializeSaveToLogFileSetting();
    initializeEnableLSPSetting();

    updateScriptPaths();
}

void OptionsDialog::addSettings(QWidget *widget)
{
    ui->stackedWidget->addWidget(widget);
    ui->listWidget->addItem(widget->windowTitle());
}

OptionsDialog::~OptionsDialog() = default;

void OptionsDialog::initializeSaveToLogFileSetting()
{
    // Save Logs to file
    ui->saveLogsToFile->setChecked(Core::Settings::instance()->value<bool>(Core::Settings::SaveLogsToFile));
    connect(ui->saveLogsToFile, &QCheckBox::toggled, this, &OptionsDialog::changeSaveLogsToFileSetting);
}

void OptionsDialog::initializeEnableLSPSetting()
{
    // Enable LSP when running in Gui mode
    ui->enableLSP->setChecked(Core::Settings::instance()->value<bool>(Core::Settings::EnableLSP));
    connect(ui->enableLSP, &QCheckBox::toggled, this, &OptionsDialog::changeEnableLSPSetting);
}

void OptionsDialog::initializeScriptPathSettings()
{
    // User and project paths settings
    ui->userPath->setText(Core::Settings::instance()->userFilePath());
    ui->openUserButton->setDisabled(ui->userPath->text().isEmpty());
    ui->projectPath->setText(Core::Settings::instance()->projectFilePath());
    ui->openProjectButton->setDisabled(ui->projectPath->text().isEmpty());

    const QString logFilePath = Core::Settings::instance()->logFilePath();
    ui->logFilePath->setText(logFilePath);
    ui->openLogFileButton->setEnabled(QFileInfo::exists(logFilePath));

    connect(ui->openUserButton, &QPushButton::clicked, this, &OptionsDialog::openUserSettings);
    connect(ui->openProjectButton, &QPushButton::clicked, this, &OptionsDialog::openProjectSettings);
    connect(ui->openLogFileButton, &QPushButton::clicked, this, &OptionsDialog::openLogFile);

    // Script paths settings
    connect(ui->addButton, &QPushButton::clicked, this, &OptionsDialog::addScriptPath);
    connect(ui->removeButton, &QPushButton::clicked, this, &OptionsDialog::removeScriptPath);
    connect(ui->scriptPathList, &QListWidget::itemSelectionChanged, this, [this]() {
        ui->removeButton->setEnabled(!ui->scriptPathList->selectedItems().isEmpty());
    });

    // Script list
    ui->scriptsList->setModel(Core::ScriptManager::model());
}

void OptionsDialog::initializeScriptBehaviorSettings()
{
    // Text editor settings
    ui->tabSize->setValidator(new QIntValidator(ui->tabSize));
    const auto settings = DEFAULT_VALUE(Core::TabSettings, Tab);
    ui->insertSpacesCheck->setChecked(settings.insertSpaces);
    ui->tabSize->setText(QString::number(settings.tabSize));

    auto changeTextEditorSettings = [this]() {
        auto settings = DEFAULT_VALUE(Core::TabSettings, Tab);
        settings.insertSpaces = ui->insertSpacesCheck->isChecked();
        settings.tabSize = ui->tabSize->text().toInt();
        Core::Settings::instance()->setValue(Core::Settings::Tab, settings);
    };
    connect(ui->insertSpacesCheck, &QCheckBox::toggled, this, changeTextEditorSettings);
    connect(ui->tabSize, &QLineEdit::textEdited, this, changeTextEditorSettings);

    // Toggle section settings
    auto sectionSettings = DEFAULT_VALUE(Core::ToggleSectionSettings, ToggleSection);
    ui->tagEdit->setText(sectionSettings.tag);
    ui->debugEdit->setText(sectionSettings.debug);
    ui->returnValues->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->returnValues->sortItems(0, Qt::AscendingOrder);
    for (const auto &[source, result] : sectionSettings.return_values) {
        auto item = new QTreeWidgetItem(ui->returnValues);
        item->setText(0, QString::fromStdString(source));
        item->setText(1, QString::fromStdString(result));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    }
    connect(ui->tagEdit, &QLineEdit::editingFinished, this, &OptionsDialog::changeToggleSectionSetting);
    connect(ui->debugEdit, &QLineEdit::editingFinished, this, &OptionsDialog::changeToggleSectionSetting);
    connect(ui->returnValues, &QTreeWidget::itemChanged, this, &OptionsDialog::changeToggleSectionSetting);

    auto addReturnValue = [this]() {
        auto item = new QTreeWidgetItem();
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        ui->returnValues->addTopLevelItem(item);
        ui->returnValues->editItem(item);
    };
    connect(ui->addReturnButton, &QPushButton::clicked, this, addReturnValue);

    auto removeReturnValue = [this]() {
        delete ui->returnValues->currentItem();
        changeToggleSectionSetting();
    };
    connect(ui->removeReturnButton, &QPushButton::clicked, this, removeReturnValue);

    connect(ui->returnValues, &QTreeWidget::itemSelectionChanged, this, [this]() {
        ui->removeReturnButton->setEnabled(!ui->returnValues->selectedItems().isEmpty());
    });
}

void OptionsDialog::initializeRcSettings()
{
    auto assetFlags = DEFAULT_VALUE(Core::RcDocument::ConversionFlags, RcAssetFlags);
    ui->removeUnknown->setChecked(assetFlags & Core::RcDocument::RemoveUnknown);
    ui->convertPng->setChecked(assetFlags & Core::RcDocument::ConvertToPng);
    ui->splitToolbars->setChecked(assetFlags & Core::RcDocument::SplitToolBar);
    connect(ui->removeUnknown, &QCheckBox::toggled, this, &OptionsDialog::changeAssetFlagsSetting);
    connect(ui->convertPng, &QCheckBox::toggled, this, &OptionsDialog::changeAssetFlagsSetting);
    connect(ui->splitToolbars, &QCheckBox::toggled, this, &OptionsDialog::changeAssetFlagsSetting);

    auto colorFlags = DEFAULT_VALUE(Core::RcDocument::ConversionFlags, RcAssetColors);
    ui->gray->setChecked(colorFlags & Core::RcDocument::Gray);
    ui->magenta->setChecked(colorFlags & Core::RcDocument::Magenta);
    ui->bottomLeft->setChecked(colorFlags & Core::RcDocument::BottomLeftPixel);
    connect(ui->gray, &QCheckBox::toggled, this, &OptionsDialog::changeAssetColorsSetting);
    connect(ui->magenta, &QCheckBox::toggled, this, &OptionsDialog::changeAssetColorsSetting);
    connect(ui->bottomLeft, &QCheckBox::toggled, this, &OptionsDialog::changeAssetColorsSetting);

    auto flags = DEFAULT_VALUE(Core::RcDocument::ConversionFlags, RcDialogFlags);
    ui->updateGeometry->setChecked(flags & Core::RcDocument::UpdateGeometry);
    ui->updateHierarchy->setChecked(flags & Core::RcDocument::UpdateHierarchy);
    ui->idForPixmap->setChecked(flags & Core::RcDocument::UseIdForPixmap);
    connect(ui->updateGeometry, &QCheckBox::toggled, this, &OptionsDialog::changeDialogFlagsSetting);
    connect(ui->updateHierarchy, &QCheckBox::toggled, this, &OptionsDialog::changeDialogFlagsSetting);
    connect(ui->idForPixmap, &QCheckBox::toggled, this, &OptionsDialog::changeDialogFlagsSetting);

    const auto scaleX = DEFAULT_VALUE(double, RcDialogScaleX);
    ui->scaleX->setValue(scaleX);
    connect(ui->scaleX, &QDoubleSpinBox::valueChanged, this, [](double value) {
        SET_DEFAULT_VALUE(RcDialogScaleX, value);
    });

    const auto scaleY = DEFAULT_VALUE(double, RcDialogScaleY);
    ui->scaleY->setValue(scaleY);
    connect(ui->scaleY, &QDoubleSpinBox::valueChanged, this, [](double value) {
        SET_DEFAULT_VALUE(RcDialogScaleY, value);
    });

    auto languageMap =
        Core::Settings::instance()->value<std::map<std::string, std::string>>(Core::Settings::RcLanguageMap);
    for (const auto &[source, result] : languageMap) {
        auto item = new QTreeWidgetItem(ui->languageMap);
        item->setText(0, QString::fromStdString(source));
        item->setText(1, QString::fromStdString(result));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    }
    connect(ui->languageMap, &QTreeWidget::itemChanged, this, &OptionsDialog::changeLanguageMap);

    auto addLanguageValue = [this]() {
        auto item = new QTreeWidgetItem();
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        ui->languageMap->addTopLevelItem(item);
        ui->languageMap->editItem(item);
    };
    connect(ui->addLanguageButton, &QPushButton::clicked, this, addLanguageValue);

    auto removeLanguageValue = [this]() {
        delete ui->languageMap->currentItem();
        changeLanguageMap();
    };
    connect(ui->removeLanguageButton, &QPushButton::clicked, this, removeLanguageValue);

    connect(ui->languageMap, &QTreeWidget::itemSelectionChanged, this, [this]() {
        ui->removeLanguageButton->setEnabled(!ui->languageMap->selectedItems().isEmpty());
    });
}

void OptionsDialog::openUserSettings()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->userPath->text()));
}

void OptionsDialog::openProjectSettings()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->projectPath->text()));
}

void OptionsDialog::openLogFile()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->logFilePath->text()));
}

void OptionsDialog::addScriptPath()
{
    const QString scriptPath = QFileDialog::getExistingDirectory(this, tr("Add Script Path"), QDir::currentPath());
    if (scriptPath.isEmpty())
        return;
    Core::ScriptManager::instance()->addDirectory(scriptPath);
    updateScriptPaths();
}

void OptionsDialog::removeScriptPath()
{
    const QString scriptPath = ui->scriptPathList->selectedItems().first()->text();
    Core::ScriptManager::instance()->removeDirectory(scriptPath);
    updateScriptPaths();
}

void OptionsDialog::updateScriptPaths()
{
    QStringList scriptPaths = Core::ScriptManager::instance()->directories();
    std::ranges::sort(scriptPaths);
    ui->scriptPathList->clear();
    ui->scriptPathList->addItems(scriptPaths);
}

void OptionsDialog::changeSaveLogsToFileSetting()
{
    SET_DEFAULT_VALUE(SaveLogsToFile, ui->saveLogsToFile->checkState() == Qt::Checked);
}

void OptionsDialog::changeEnableLSPSetting()
{
    SET_DEFAULT_VALUE(EnableLSP, ui->enableLSP->checkState() == Qt::Checked);
}

void OptionsDialog::changeToggleSectionSetting()
{
    auto sectionSettings = DEFAULT_VALUE(Core::ToggleSectionSettings, ToggleSection);
    std::map<std::string, std::string> returnValues;
    for (int i = 0; i < ui->returnValues->topLevelItemCount(); ++i) {
        auto item = ui->returnValues->topLevelItem(i);
        if (item->text(0).isEmpty() || item->text(1).isEmpty())
            continue;
        returnValues[item->text(0).toStdString()] = item->text(1).toStdString();
    }

    if (sectionSettings.tag == ui->tagEdit->text() && sectionSettings.debug == ui->debugEdit->text()
        && sectionSettings.return_values == returnValues)
        return;
    sectionSettings.tag = ui->tagEdit->text();
    sectionSettings.debug = ui->debugEdit->text();
    sectionSettings.return_values = returnValues;
    Core::Settings::instance()->setValue(Core::Settings::ToggleSection, sectionSettings);
}

void OptionsDialog::changeAssetFlagsSetting()
{
    Core::RcDocument::ConversionFlags flags = Core::RcDocument::NoFlags;
    if (ui->removeUnknown->isChecked())
        flags |= Core::RcDocument::RemoveUnknown;
    if (ui->convertPng->isChecked())
        flags |= Core::RcDocument::ConvertToPng;
    if (ui->splitToolbars->isChecked())
        flags |= Core::RcDocument::SplitToolBar;
    SET_DEFAULT_VALUE(RcAssetFlags, flags);
}

void OptionsDialog::changeAssetColorsSetting()
{
    Core::RcDocument::ConversionFlags colors = Core::RcDocument::NoColors;
    if (ui->gray->isChecked())
        colors |= Core::RcDocument::Gray;
    if (ui->magenta->isChecked())
        colors |= Core::RcDocument::Magenta;
    if (ui->bottomLeft->isChecked())
        colors |= Core::RcDocument::BottomLeftPixel;
    SET_DEFAULT_VALUE(RcAssetColors, colors);
}

void OptionsDialog::changeDialogFlagsSetting()
{
    Core::RcDocument::ConversionFlags flags = Core::RcDocument::NoFlags;
    if (ui->updateGeometry->isChecked())
        flags |= Core::RcDocument::UpdateGeometry;
    if (ui->updateHierarchy->isChecked())
        flags |= Core::RcDocument::UpdateHierarchy;
    if (ui->idForPixmap->isChecked())
        flags |= Core::RcDocument::UseIdForPixmap;
    SET_DEFAULT_VALUE(RcDialogFlags, flags);
}

void OptionsDialog::changeLanguageMap()
{
    auto oldLanguageMap =
        Core::Settings::instance()->value<std::map<std::string, std::string>>(Core::Settings::RcLanguageMap);
    std::map<std::string, std::string> languageMap;
    for (int i = 0; i < ui->languageMap->topLevelItemCount(); ++i) {
        auto item = ui->languageMap->topLevelItem(i);
        if (item->text(0).isEmpty())
            continue;
        const std::string result =
            item->text(1).isEmpty() ? Core::RcDocument::DefaultLanguage : item->text(1).toStdString();
        languageMap[item->text(0).toStdString()] = result;
    }
    Core::Settings::instance()->setValue(Core::Settings::RcLanguageMap, languageMap);
}

void OptionsDialog::changePage()
{
    ui->stackedWidget->setCurrentIndex(ui->listWidget->currentRow());
    ui->title->setText(ui->listWidget->currentItem()->text());
}

} // namespace Gui
