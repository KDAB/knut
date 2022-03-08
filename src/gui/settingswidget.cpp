#include "settingswidget.h"
#include "ui_settingswidget.h"

#include "core/scriptmanager.h"
#include "core/settings.h"

#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QUrl>

#include <algorithm>

namespace Gui {

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);
    setWindowTitle(QApplication::applicationName() + ' ' + QApplication::applicationVersion() + " - " + windowTitle());

    connect(ui->openUserButton, &QPushButton::clicked, this, &SettingsWidget::openUserSettings);
    connect(ui->openProjectButton, &QPushButton::clicked, this, &SettingsWidget::openProjectSettings);
    connect(ui->addButton, &QPushButton::clicked, this, &SettingsWidget::addScriptPath);
    connect(ui->removeButton, &QPushButton::clicked, this, &SettingsWidget::removeScriptPath);

    ui->userPath->setText(Core::Settings::instance()->userFilePath());
    ui->openUserButton->setDisabled(ui->userPath->text().isEmpty());
    ui->projectPath->setText(Core::Settings::instance()->projectFilePath());
    ui->openProjectButton->setDisabled(ui->projectPath->text().isEmpty());

    connect(ui->scriptPathList, &QListWidget::itemSelectionChanged, this, [this]() {
        ui->removeButton->setEnabled(ui->scriptPathList->selectedItems().size());
    });

    updateScriptPaths();
}

SettingsWidget::~SettingsWidget() = default;

void SettingsWidget::openUserSettings()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->userPath->text()));
}

void SettingsWidget::openProjectSettings()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->projectPath->text()));
}

void SettingsWidget::addScriptPath()
{
    const QString scriptPath = QFileDialog::getExistingDirectory(this, "Add Script Path", QDir::currentPath());
    if (scriptPath.isEmpty())
        return;
    Core::ScriptManager::instance()->addDirectory(scriptPath);
    updateScriptPaths();
}

void SettingsWidget::removeScriptPath()
{
    const QString scriptPath = ui->scriptPathList->selectedItems().first()->text();
    Core::ScriptManager::instance()->removeDirectory(scriptPath);
    updateScriptPaths();
}

void SettingsWidget::updateScriptPaths()
{
    QStringList scriptPaths = Core::ScriptManager::instance()->directories();
    std::sort(scriptPaths.begin(), scriptPaths.end());
    ui->scriptPathList->clear();
    ui->scriptPathList->addItems(scriptPaths);
}

} // namespace Gui
