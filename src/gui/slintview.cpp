#include "slintview.h"
#include "core/slintdocument.h"
#include "core/logger.h"

#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QToolBar>
#include <QStandardPaths>
#include <QProcess>

namespace Gui {

SlintView::SlintView(QWidget* parent):
    QWidget(parent),
    m_process(new QProcess(this))
{
}

SlintView::~SlintView()
{
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->terminate();
        m_process->waitForFinished(300);
    }
}

static QString findSlintViewer()
{
#if defined(Q_OS_WIN)
    return QStandardPaths::findExecutable("slint-viewer.exe");
#else
    if (auto path = QStandardPaths::findExecutable("slint-viewer"); path.isEmpty()) {
        QStringList paths;
        for(const auto& home: QStandardPaths::standardLocations(QStandardPaths::HomeLocation)) {
            paths.append(home+"/.cargo/bin/");
        }
        path = QStandardPaths::findExecutable("slint-viewer", paths);
        return path;
    } else {
        return path;
    }
#endif
}

void SlintView::setSlintDocument(Core::SlintDocument *document)
{
    m_document = document;
    auto lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    auto exec = findSlintViewer();

    QAction* act = new QAction(tr("Run"));
    act->setDisabled(exec.isEmpty());
    connect(act, &QAction::triggered, this, &SlintView::runSlint);

    auto bar = new QToolBar(this);
    bar->addAction(act);
    lay->addWidget(bar);

    auto edit = document->textEdit();
    edit->setVisible(true);
    lay->addWidget(edit, 1);
}

void SlintView::runSlint()
{
    if (m_process && m_process->state() == QProcess::Running) {
        spdlog::warn("Slint-viewer process is already running");
        return;
    }

    if (!m_document) {
        return;
    }

    auto exec = findSlintViewer();

    if (exec.isEmpty()) {
        return;
    }

    m_process->start(exec, {m_document->fileName()});
}



}
