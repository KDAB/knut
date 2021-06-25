#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "core/scriptmanager.h"
#include "core/settings.h"

#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QUrl>

#include <algorithm>

namespace Gui {

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setWindowTitle(QApplication::applicationName() + ' ' + QApplication::applicationVersion() + " - " + windowTitle());

    connect(ui->openUserButton, &QPushButton::clicked, this, &SettingsDialog::openUserSettings);
    connect(ui->openProjectButton, &QPushButton::clicked, this, &SettingsDialog::openProjectSettings);
    connect(ui->addButton, &QPushButton::clicked, this, &SettingsDialog::addScriptPath);
    connect(ui->removeButton, &QPushButton::clicked, this, &SettingsDialog::removeScriptPath);

    ui->userPath->setText(Core::Settings::instance()->userPath());
    ui->openUserButton->setDisabled(ui->userPath->text().isEmpty());
    ui->projectPath->setText(Core::Settings::instance()->projectPath());
    ui->openProjectButton->setDisabled(ui->projectPath->text().isEmpty());

    connect(ui->scriptPathList, &QListWidget::itemSelectionChanged, this, [this]() {
        ui->removeButton->setEnabled(ui->scriptPathList->selectedItems().size());
    });

    updateScriptPaths();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::openUserSettings()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->userPath->text()));
}

void SettingsDialog::openProjectSettings()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->projectPath->text()));
}

void SettingsDialog::addScriptPath()
{
    const QString scriptPath = QFileDialog::getExistingDirectory(this, "Add Script Path", QDir::currentPath());
    if (scriptPath.isEmpty())
        return;
    Core::Settings::instance()->addScriptPath(scriptPath);
    updateScriptPaths();
}

void SettingsDialog::removeScriptPath()
{
    const QString scriptPath = ui->scriptPathList->selectedItems().first()->text();
    Core::Settings::instance()->removeScriptPath(scriptPath);
    updateScriptPaths();
}

void SettingsDialog::updateScriptPaths()
{
    QStringList scriptPaths = Core::ScriptManager::instance()->directories();
    std::sort(scriptPaths.begin(), scriptPaths.end());
    ui->scriptPathList->clear();
    ui->scriptPathList->addItems(scriptPaths);
}

} // namespace Gui
