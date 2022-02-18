#include "runscriptdialog.h"
#include "ui_runscriptdialog.h"

#include "core/scriptmanager.h"

#include <QApplication>
#include <QCompleter>
#include <QFileDialog>
#include <QPushButton>

#include <algorithm>

namespace Gui {

RunScriptDialog::RunScriptDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RunScriptDialog)
{
    ui->setupUi(this);
    setWindowTitle(QApplication::applicationName() + ' ' + QApplication::applicationVersion() + " - " + windowTitle());

    connect(ui->toolButton, &QToolButton::clicked, this, &RunScriptDialog::chooseScript);
    auto okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(false);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, [okButton](const QString &str) {
        okButton->setEnabled(!str.trimmed().isEmpty());
    });

    auto list = Core::ScriptManager::instance()->scriptList();
    QStringList scriptNames;
    scriptNames.reserve(static_cast<int>(list.size()));
    for (const auto &item : list)
        scriptNames.push_back(item.name);
    auto completer = new QCompleter(scriptNames, this);
    ui->lineEdit->setCompleter(completer);
}

RunScriptDialog::~RunScriptDialog() = default;

void RunScriptDialog::accept()
{
    if (ui->lineEdit->text().isEmpty())
        return;

    QString scriptName = ui->lineEdit->text();
    QFileInfo fi(scriptName);
    if (!fi.exists()) {
        auto list = Core::ScriptManager::instance()->scriptList();
        auto it = std::find_if(list.cbegin(), list.cend(), [&scriptName](const auto &item) {
            return item.name == scriptName;
        });
        if (it == list.cend())
            return;
        scriptName = it->fileName;
    }

    Core::ScriptManager::instance()->runScript(scriptName);
    QDialog::accept();
}

void RunScriptDialog::chooseScript()
{
    const QString fileName =
        QFileDialog::getOpenFileName(this, QString(), ui->lineEdit->text(), tr("Scripts (*.js *.qml)"));
    if (fileName.isEmpty())
        return;
    ui->lineEdit->setText(fileName);
}

} // namespace Gui
