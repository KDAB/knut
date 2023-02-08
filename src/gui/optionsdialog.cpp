#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include "core/cppdocument_p.h"
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

    // User and project paths settings
    ui->userPath->setText(Core::Settings::instance()->userFilePath());
    ui->openUserButton->setDisabled(ui->userPath->text().isEmpty());
    ui->projectPath->setText(Core::Settings::instance()->projectFilePath());
    ui->openProjectButton->setDisabled(ui->projectPath->text().isEmpty());
    connect(ui->openUserButton, &QPushButton::clicked, this, &OptionsDialog::openUserSettings);
    connect(ui->openProjectButton, &QPushButton::clicked, this, &OptionsDialog::openProjectSettings);

    // Script paths settings
    connect(ui->addButton, &QPushButton::clicked, this, &OptionsDialog::addScriptPath);
    connect(ui->removeButton, &QPushButton::clicked, this, &OptionsDialog::removeScriptPath);
    connect(ui->scriptPathList, &QListWidget::itemSelectionChanged, this, [this]() {
        ui->removeButton->setEnabled(ui->scriptPathList->selectedItems().size());
    });

    // Json paths settings
    connect(ui->addJsonButton, &QPushButton::clicked, this, &OptionsDialog::addJsonPath);
    connect(ui->removeJsonButton, &QPushButton::clicked, this, &OptionsDialog::removeJsonPath);
    connect(ui->jsonPathList, &QListWidget::itemSelectionChanged, this, [this]() {
        ui->removeJsonButton->setEnabled(ui->jsonPathList->selectedItems().size());
    });

    // Text editor settings
    ui->tabSize->setValidator(new QIntValidator(ui->tabSize));
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

    // Toggle section settings
    auto sectionSettings =
        Core::Settings::instance()->value<Core::ToggleSectionSettings>(Core::Settings::ToggleSection);
    ui->tagEdit->setText(sectionSettings.tag);
    ui->debugEdit->setText(sectionSettings.debug);
    ui->returnValues->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->returnValues->sortItems(0, Qt::AscendingOrder);
    for (const auto &returnValue : sectionSettings.return_values) {
        auto item = new QTreeWidgetItem(ui->returnValues);
        item->setText(0, QString::fromStdString(returnValue.first));
        item->setText(1, QString::fromStdString(returnValue.second));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    }
    connect(ui->tagEdit, &QLineEdit::editingFinished, this, &OptionsDialog::changeToggleSectionSetting);
    connect(ui->debugEdit, &QLineEdit::editingFinished, this, &OptionsDialog::changeToggleSectionSetting);
    connect(ui->returnValues, &QTreeWidget::itemChanged, this, &OptionsDialog::changeToggleSectionSetting);
    connect(ui->addReturnButton, &QPushButton::clicked, this, &OptionsDialog::addReturnValue);
    connect(ui->removeReturnButton, &QPushButton::clicked, this, &OptionsDialog::removeReturnValue);
    connect(ui->returnValues, &QTreeWidget::itemSelectionChanged, this, [this]() {
        ui->removeReturnButton->setEnabled(ui->returnValues->selectedItems().size());
    });

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
    std::ranges::sort(scriptPaths);
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
    std::ranges::sort(jsonPaths);
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

void OptionsDialog::addReturnValue()
{
    auto item = new QTreeWidgetItem();
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    ui->returnValues->addTopLevelItem(item);
    ui->returnValues->editItem(item);
}

void OptionsDialog::removeReturnValue()
{
    delete ui->returnValues->currentItem();
    changeToggleSectionSetting();
}

void OptionsDialog::changeToggleSectionSetting()
{
    auto sectionSettings =
        Core::Settings::instance()->value<Core::ToggleSectionSettings>(Core::Settings::ToggleSection);
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

void OptionsDialog::changePage()
{
    ui->stackedWidget->setCurrentIndex(ui->listWidget->currentRow());
    ui->title->setText(ui->listWidget->currentItem()->text());
}

} // namespace Gui
