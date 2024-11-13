/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "findwidget.h"
#include "core/logger.h"
#include "core/project.h"
#include "core/qttsdocument.h"
#include "core/qtuidocument.h"
#include "core/textdocument.h"
#include "guisettings.h"
#include "qttsview.h"
#include "qtuiview.h"
#include "ui_findwidget.h"

#include <QAction>
#include <QKeySequence>
#include <QPlainTextEdit>
#include <QTextDocument>

namespace Gui {

FindWidget::FindWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FindWidget)
{
    ui->setupUi(this);
    setProperty("panelWidget", true);
    setFocusProxy(ui->findEdit);

    auto createAction = [this](const QString &text, const QString &icon, const QString &shortcut, QLineEdit *lineEdit) {
        auto action = new QAction(text, this);
        action->setCheckable(true);
        GuiSettings::setIcon(action, icon);
        action->setShortcut(shortcut);
        action->setShortcutContext(Qt::WidgetShortcut);
        lineEdit->addAction(action, QLineEdit::TrailingPosition);
        return action;
    };
    m_matchRegexp = createAction("Match Regular Expression", ":/gui/regex.png", "Alt+Shift+R", ui->findEdit);
    m_matchWord = createAction("Match Match Whole Word", ":/gui/whole-word.png", "Alt+Shift+W", ui->findEdit);
    m_matchCase = createAction("Match Case", ":/gui/format-letter-case.png", "Alt+Shift+C", ui->findEdit);
    m_preserveCase = createAction("Preserve Case", ":/gui/format-letter-case.png", "Alt+Shift+C", ui->replaceEdit);

    GuiSettings::setIcon(ui->closeButton, ":/gui/close.png");
    connect(ui->closeButton, &QToolButton::clicked, this, &QWidget::hide);

    connect(ui->findEdit, &QLineEdit::returnPressed, this, &FindWidget::findNext);
    connect(ui->nextButton, &QToolButton::pressed, this, &FindWidget::findNext);
    connect(ui->previousButton, &QToolButton::pressed, this, &FindWidget::findPrevious);
    connect(ui->replaceEdit, &QLineEdit::returnPressed, this, &FindWidget::replaceOne);
    connect(ui->replaceButton, &QToolButton::pressed, this, &FindWidget::replaceOne);
    connect(ui->replaceAllbutton, &QToolButton::pressed, this, &FindWidget::replaceAll);
}

FindWidget::~FindWidget() = default;

int FindWidget::findFlags() const
{
    return (m_matchCase->isChecked() ? Core::TextDocument::FindCaseSensitively : Core::TextDocument::NoFindFlags)
        | (m_matchWord->isChecked() ? Core::TextDocument::FindWholeWords : Core::TextDocument::NoFindFlags)
        | (m_matchRegexp->isChecked() ? Core::TextDocument::FindRegexp : Core::TextDocument::NoFindFlags)
        | (m_preserveCase->isChecked() ? Core::TextDocument::PreserveCase : Core::TextDocument::NoFindFlags);
}

QString FindWidget::findString()
{
    const QString &text = ui->findEdit->text();

    // Enable logging and replay of selected text or word, only the first time
    // Following find or replace won't call this
    if (m_firstTime) {
        auto textDocument = qobject_cast<Core::TextDocument *>(Core::Project::instance()->currentDocument());
        if (text == m_defaultString) {
            if (m_isDefaultSelection)
                textDocument->selectedText();
            else
                textDocument->currentWord();
        }
        m_firstTime = false;
    }
    return text;
}

void FindWidget::findNext()
{
    find(findFlags());
}

void FindWidget::findPrevious()
{
    find(findFlags() | Core::TextDocument::FindBackward);
}

void FindWidget::open()
{
    // Store the current selection or current word, so if it's what is used for the find or replace
    // we can call the following API of TextDocument for logging and replay
    auto document = Core::Project::instance()->currentDocument();
    if (auto textDocument = qobject_cast<Core::TextDocument *>(document)) {
        Core::LoggerDisabler ld;
        m_defaultString = textDocument->selectedText();
        m_isDefaultSelection = true;
        if (m_defaultString.isEmpty()) {
            m_defaultString = textDocument->currentWord();
            m_isDefaultSelection = false;
        }
        ui->findEdit->setText(m_defaultString);
        ui->findEdit->selectAll();
    } else {
        ui->findEdit->clear();
    }
    ui->replaceEdit->clear();
    m_firstTime = true;
    show();
    ui->findEdit->setFocus(Qt::OtherFocusReason);
}

void FindWidget::find(int options)
{
    if (ui->findEdit->text().isEmpty())
        return;
    Q_EMIT findRequested(ui->findEdit->text(), options);
}

void FindWidget::replaceOne()
{
    replace(false);
}

void FindWidget::replaceAll()
{
    replace(true);
}

void FindWidget::replace(bool replaceAll)
{
    const QString &before = findString();
    const QString &after = ui->replaceEdit->text();
    if (before.isEmpty())
        return;

    Q_EMIT replaceRequested(before, after, findFlags(), replaceAll);
}

void FindWidget::hideEvent(QHideEvent *event)
{
    Q_EMIT widgetClosed();
    QWidget::hideEvent(event);
}

void FindWidget::setReplaceVisible(bool show)
{
    // We don't want to use a wrapping frame here due to layouting issues...
    ui->replaceWithLabel->setVisible(show);
    ui->replaceEdit->setVisible(show);
    ui->replaceButtonWidget->setVisible(show);
}

} // namespace Gui
