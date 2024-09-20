/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "slintview.h"
#include "core/textdocument.h"
#include "guisettings.h"
#include "utils/log.h"

#include <QPlainTextEdit>
#include <QProcess>
#include <QStandardPaths>
#include <QToolBar>
#include <QVBoxLayout>

namespace Gui {

static QString findSlintViewer()
{
#if defined(Q_OS_WIN)
    return QStandardPaths::findExecutable("slint-viewer.exe");
#else
    if (auto path = QStandardPaths::findExecutable("slint-viewer"); path.isEmpty()) {
        QStringList paths;
        const auto standarLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        for (const auto &home : standarLocations) {
            paths.append(home + "/.cargo/bin/");
        }
        path = QStandardPaths::findExecutable("slint-viewer", paths);
        return path;
    } else {
        return path;
    }
#endif
}

SlintView::SlintView(QWidget *parent)
    : TextView(parent)
    , m_process(new QProcess(this))
{
    const auto exec = findSlintViewer();

    auto *action = new QAction(tr("Run"), this);
    GuiSettings::setIcon(action, ":/gui/eye.png");
    action->setDisabled(exec.isEmpty());
    if (exec.isEmpty()) {
        action->setToolTip(tr("slint-viewer executable wasn't found. Install it using: cargo install slint-viewer"));
    }
    connect(action, &QAction::triggered, this, &SlintView::runSlint);
    addAction(action);
}

SlintView::~SlintView()
{
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->terminate();
        m_process->waitForFinished(300);
    }
}

void SlintView::runSlint()
{
    if (m_process && m_process->state() == QProcess::Running) {
        spdlog::warn("{}: Slint-viewer process is already running", FUNCTION_NAME);
        return;
    }

    if (!document()) {
        return;
    }

    const auto exec = findSlintViewer();

    if (exec.isEmpty()) {
        return;
    }

    if (m_process) {
        m_process->start(exec, {document()->fileName()});
    }
}

}
