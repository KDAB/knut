#include "settingswidget.h"
#include "ui_settingswidget.h"

#include "core/project.h"
#include "core/scriptmanager.h"
#include "core/settings.h"
#include "core/textdocument_p.h"

#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QIntValidator>
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
    connect(ui->addJsonButton, &QPushButton::clicked, this, &SettingsWidget::addJsonPath);
    connect(ui->removeJsonButton, &QPushButton::clicked, this, &SettingsWidget::removeJsonPath);

    ui->userPath->setText(Core::Settings::instance()->userFilePath());
    ui->openUserButton->setDisabled(ui->userPath->text().isEmpty());
    ui->projectPath->setText(Core::Settings::instance()->projectFilePath());
    ui->openProjectButton->setDisabled(ui->projectPath->text().isEmpty());

    connect(ui->scriptPathList, &QListWidget::itemSelectionChanged, this, [this]() {
        ui->removeButton->setEnabled(ui->scriptPathList->selectedItems().size());
    });
    connect(ui->jsonPathList, &QListWidget::itemSelectionChanged, this, [this]() {
        ui->removeJsonButton->setEnabled(ui->jsonPathList->selectedItems().size());
    });

    ui->tabSize->setValidator(new QIntValidator(ui->tabSize));

    ui->textEditorGroup->setDisabled(Core::Project::instance()->root().isEmpty());
    const auto settings = Core::Settings::instance()->value<Core::TabSettings>(Core::Settings::Tab);
    ui->insertSpacesCheck->setChecked(settings.insertSpaces);
    ui->tabSize->setText(QString::number(settings.tabSize));

    auto changeTextEditorSettings = [this]() {
        auto settings = Core::Settings::instance()->value<Core::TabSettings>(Core::Settings::Tab);
        settings.insertSpaces = ui->insertSpacesCheck->isChecked();
        settings.tabSize = ui->tabSize->text().toInt();
        Core::Settings::instance()->setValue(Core::Settings::Tab, settings);
    };
    connect(ui->insertSpacesCheck, &QCheckBox::toggled, this, changeTextEditorSettings);
    connect(ui->tabSize, &QLineEdit::textEdited, this, changeTextEditorSettings);

    updateScriptPaths();
    updateJsonPaths();
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
    const QString scriptPath = QFileDialog::getExistingDirectory(this, tr("Add Script Path"), QDir::currentPath());
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

void SettingsWidget::addJsonPath()
{
    const QString jsonPath = QFileDialog::getExistingDirectory(this, tr("Add Json Path"), QDir::currentPath());
    if (jsonPath.isEmpty())
        return;
    Core::Settings::instance()->addJsonPath(jsonPath);
    updateJsonPaths();
}

void SettingsWidget::removeJsonPath()
{
    const QString jsonPath = ui->jsonPathList->selectedItems().first()->text();
    Core::Settings::instance()->removeJsonPath(jsonPath);
    updateJsonPaths();
}

void SettingsWidget::updateJsonPaths()
{
    QStringList jsonPaths = Core::Settings::instance()->value<QStringList>(Core::Settings::JsonPaths);
    std::sort(jsonPaths.begin(), jsonPaths.end());
    ui->jsonPathList->clear();
    for (const auto &path : jsonPaths) {
        if (path.startsWith(":/")) {
            auto item = new QListWidgetItem(tr("<internal>"), ui->jsonPathList);
            item->setFlags(Qt::ItemIsEnabled);
        } else {
            ui->jsonPathList->addItem(path);
        }
    }
}

} // namespace Gui
