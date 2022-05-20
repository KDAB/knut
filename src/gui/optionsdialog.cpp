#include "optionsdialog.h"
#include "ui_optionsdialog.h"

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

    connect(ui->openUserButton, &QPushButton::clicked, this, &OptionsDialog::openUserSettings);
    connect(ui->openProjectButton, &QPushButton::clicked, this, &OptionsDialog::openProjectSettings);
    connect(ui->addButton, &QPushButton::clicked, this, &OptionsDialog::addScriptPath);
    connect(ui->removeButton, &QPushButton::clicked, this, &OptionsDialog::removeScriptPath);
    connect(ui->addJsonButton, &QPushButton::clicked, this, &OptionsDialog::addJsonPath);
    connect(ui->removeJsonButton, &QPushButton::clicked, this, &OptionsDialog::removeJsonPath);

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

void OptionsDialog::addSettings(QWidget *widget)
{
    ui->stackedWidget->addWidget(widget);
    ui->listWidget->addItem(widget->windowTitle());
}

OptionsDialog::~OptionsDialog() = default;

void OptionsDialog::openUserSettings()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->userPath->text()));
}

void OptionsDialog::openProjectSettings()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->projectPath->text()));
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
    std::sort(scriptPaths.begin(), scriptPaths.end());
    ui->scriptPathList->clear();
    ui->scriptPathList->addItems(scriptPaths);
}

void OptionsDialog::addJsonPath()
{
    const QString jsonPath = QFileDialog::getExistingDirectory(this, tr("Add Json Path"), QDir::currentPath());
    if (jsonPath.isEmpty())
        return;
    Core::Settings::instance()->addJsonPath(jsonPath);
    updateJsonPaths();
}

void OptionsDialog::removeJsonPath()
{
    const QString jsonPath = ui->jsonPathList->selectedItems().first()->text();
    Core::Settings::instance()->removeJsonPath(jsonPath);
    updateJsonPaths();
}

void OptionsDialog::updateJsonPaths()
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

void OptionsDialog::changePage()
{
    ui->stackedWidget->setCurrentIndex(ui->listWidget->currentRow());
    ui->title->setText(ui->listWidget->currentItem()->text());
}

} // namespace Gui
