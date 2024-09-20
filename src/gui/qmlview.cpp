/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qmlview.h"
#include "core/textdocument.h"
#include "guisettings.h"

#include <QAction>
#include <QFile>
#include <QLabel>
#include <QPlainTextEdit>
#include <QQuickView>
#include <QToolBar>
#include <QVBoxLayout>

namespace Gui {

QmlView::QmlView(QWidget *parent)
    : TextView(parent)
{
    auto *action = new QAction(tr("Run"));
    GuiSettings::setIcon(action, ":/gui/eye.png");
    connect(action, &QAction::triggered, this, &QmlView::runQml);
    addAction(action);
}

QmlView::~QmlView() = default;

void QmlView::runQml()
{
    if (!document()) {
        return;
    }

    const QString qmlFilePath = document()->fileName();

    auto *qmlView = new QQuickView();
    qmlView->setSource(QUrl::fromLocalFile(qmlFilePath));
    qmlView->show();
}

} // namespace Gui
