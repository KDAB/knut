#include "scriptpanel.h"

#include "guisettings.h"

#include "core/logger.h"
#include "core/scriptmanager.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QTemporaryFile>
#include <QToolButton>

#include <spdlog/spdlog.h>

namespace Gui {

constexpr char Untitled[] = "<untitled>";

ScriptPanel::ScriptPanel(QWidget *parent)
    : QPlainTextEdit {parent}
    , m_toolBar(new QWidget)
{
    setWindowTitle(tr("Script Panel"));
    setObjectName("ScriptPanel");

    GuiSettings::setupDocumentTextEdit(this, "script.qml");

    // Setup titlebar
    auto layout = new QHBoxLayout(m_toolBar);
    layout->setContentsMargins({});

    auto createButton = [this, layout](const QString &icon, const QString &tooltip) {
        auto button = new QToolButton(m_toolBar);
        GuiSettings::setIcon(button, icon);
        button->setToolTip(tooltip);
        button->setAutoRaise(true);
        layout->addWidget(button);
        return button;
    };

    auto newButton = createButton(":/gui/file.png", tr("New"));
    connect(newButton, &QToolButton::clicked, this, &ScriptPanel::newScript);

    auto openButton = createButton(":/gui/folder-open.png", tr("Open"));
    connect(openButton, &QToolButton::clicked, this, &ScriptPanel::openScript);

    auto saveButton = createButton(":/gui/content-save.png", tr("Save"));
    connect(saveButton, &QToolButton::clicked, this, &ScriptPanel::saveScript);

    auto separator = new QFrame(m_toolBar);
    separator->setFrameShape(QFrame::VLine);
    layout->addWidget(separator);

    m_scriptName = new QLabel(tr(Untitled));
    layout->addWidget(m_scriptName);

    auto playButton = createButton(":/gui/play.png", tr("Play"));
    connect(playButton, &QToolButton::clicked, this, &ScriptPanel::playScript);

    layout->addSpacing(50);
}

QWidget *ScriptPanel::toolBar() const
{
    return m_toolBar;
}

void ScriptPanel::setNewScript(const QString &script)
{
    newScript();
    setPlainText(script);
}

void ScriptPanel::openScript()
{
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Open Script"), "", "Script files (*.qml *.js)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        setPlainText(file.readAll());
        m_fileName = fileName;
        QFileInfo fi(m_fileName);
        m_scriptName->setText(fi.fileName());
    } else {
        spdlog::error("Error reading script - can't open the file for reading.");
    }
}

void ScriptPanel::newScript()
{
    m_scriptName->setText(tr(Untitled));
    m_fileName.clear();
    clear();
}

void ScriptPanel::saveScript()
{
    if (m_fileName.isEmpty()) {
        m_fileName = QFileDialog::getSaveFileName(this, tr("Save Script"), "", "Script files (*.qml *.js)");
        if (m_fileName.isEmpty())
            return;
        QFileInfo fi(m_fileName);
        m_scriptName->setText(fi.fileName());
    }

    QFile file(m_fileName);
    if (file.open(QIODevice::WriteOnly))
        file.write(toPlainText().toUtf8());
    else
        spdlog::error("Error saving script - can't open the file for writing.");
}

void ScriptPanel::playScript()
{
    Core::LoggerDisabler ld;

    if (!m_fileName.isEmpty()) {
        Core::ScriptManager::instance()->runScript(m_fileName, false, false);
        return;
    }

    const QString extension = find("import Script") ? "qml" : "js";

    QTemporaryFile file("script_XXXXXX." + extension);
    if (file.open()) {
        file.write(toPlainText().toUtf8());
        file.close();
        Core::ScriptManager::instance()->runScript(file.fileName(), false, false);
    } else {
        spdlog::error("Error running script - can't save to temporary file.");
    }
}

} // namespace Gui
