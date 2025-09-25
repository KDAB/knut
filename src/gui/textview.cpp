/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "textview.h"
#include "core/codedocument.h"
#include "core/logger.h"
#include "core/mark.h"
#include "core/textdocument.h"
#include "guisettings.h"

#include <QEvent>
#include <QMenu>
#include <QPainter>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTextDocument>
#include <QToolButton>
#include <QToolTip>
#include <QVBoxLayout>
#include <core/settings.h>

namespace Gui {

constexpr int MarkWidth = 6;
constexpr int MarkWidth2 = MarkWidth / 2;

class MarkRect : public QWidget
{
public:
    explicit MarkRect(QWidget *parent)
        : QWidget(parent)
    {
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.translate(0.5, 0);
        const QColor &color = palette().highlight().color();
        p.setPen(color);
        p.setBrush(color);

        const QRect r = rect();
        QPolygonF p1({r.topLeft(),
                      r.topRight(),
                      {static_cast<float>(r.center().x()), static_cast<float>(r.top() + MarkWidth2 + 1)}});
        p.drawPolygon(p1);

        QPolygonF p2({r.bottomLeft(),
                      r.bottomRight(),
                      {static_cast<float>(r.center().x()), static_cast<float>(r.bottom() - MarkWidth2 - 1)}});
        p.drawPolygon(p2);

        p.setOpacity(0.7);
        p.drawLine(p1.at(2), p2.at(2));
    }
};

TextView::TextView(QWidget *parent)
    : QWidget {parent}
    , FindInterface(FindInterface::CanSearch | FindInterface::CanReplace)
    , m_quickActionButton(new QToolButton(this))
{
    m_quickActionButton->hide();
    m_quickActionButton->setIcon(QIcon(":/gui/lightbulb.png"));
    m_quickActionButton->setAutoRaise(true);
    m_quickActionButton->setFixedSize(fontMetrics().height(), fontMetrics().height());
    connect(m_quickActionButton, &QToolButton::clicked, this, &TextView::showQuickActionMenu);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void TextView::setDocument(Core::TextDocument *document)
{
    m_document = document;
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins({});

    auto *textEdit = m_document->textEdit();
    layout->addWidget(textEdit);
    textEdit->setVisible(true);
    textEdit->installEventFilter(this);
    setFocusProxy(textEdit);
    connect(textEdit->document(), &QTextDocument::contentsChanged, this, &TextView::updateMarkRect);
    GuiSettings::setupDocumentTextEdit(textEdit, document);

    connect(textEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &TextView::updateQuickActionRect);

    // TODO, change once we have quick actions
    // m_quickActionButton->raise();
}

void TextView::toggleMark()
{
    Q_ASSERT(m_document);

    const bool newMark = !m_mark || m_mark->position() != m_document->position();

    m_mark.reset();
    delete m_markRect;
    m_markRect = nullptr;

    if (newMark) {
        m_mark = m_document->createMark();
        m_markRect = new MarkRect(m_document->textEdit());
        m_markRect->show();
        updateMarkRect();
    }
}

void TextView::gotoMark()
{
    Q_ASSERT(m_document);
    if (m_mark.has_value()) {
        m_document->gotoMark(m_mark.value());
    }
}

void TextView::selectToMark()
{
    Q_ASSERT(m_document);
    if (m_mark.has_value()) {
        m_document->selectToMark(m_mark.value());
    }
}

bool TextView::hasMark() const
{
    return m_mark.has_value();
}

bool TextView::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)
    if (event->type() == QEvent::Paint)
        updateMarkRect();
    if (event->type() == QEvent::ToolTip) {
        if (DEFAULT_VALUE(bool, EnableLSP)) {
            if (const auto *codedocument = qobject_cast<Core::CodeDocument *>(m_document)) {
                if (const auto *helpEvent = dynamic_cast<QHelpEvent *>(event)) {
                    auto cursor = codedocument->textEdit()->cursorForPosition(helpEvent->pos());

                    // Make the textEdit a guarded pointer, as it might have been destroyed once the hover
                    // callback returns.
                    QPointer<QPlainTextEdit> textEdit(codedocument->textEdit());
                    QPoint position(helpEvent->globalPos());

                    // Hover spams the log if it doesn't find anything.
                    // In our case, that's not a problem, so just disable the log.
                    Core::LoggerDisabler ld;

                    codedocument->hover(cursor.position(), [textEdit, position](const auto &hoverText) {
                        if (!textEdit.isNull() && textEdit->isVisible()) {
                            QToolTip::showText(position, hoverText, textEdit);
                        }
                    });
                    return true;
                }
            }
        }
    }
    return false;
}

void TextView::find(const QString &text, int options)
{
    document()->find(text, static_cast<Core::TextDocument::FindFlags>(options));
}

void TextView::replace(const QString &before, const QString &after, int options, bool replaceAll)
{
    if (replaceAll) {
        document()->replaceAll(before, after, static_cast<Core::TextDocument::FindFlags>(options));

    } else {
        document()->replaceOne(before, after, static_cast<Core::TextDocument::FindFlags>(options));
    }
}

Core::TextDocument *TextView::document() const
{
    return m_document;
}

void TextView::updateMarkRect()
{
    if (m_markRect) {
        auto textCursor = m_document->textEdit()->textCursor();
        textCursor.setPosition(m_mark->position());
        auto rect = m_document->textEdit()->cursorRect(textCursor);
        auto viewport = m_document->textEdit()->viewport();
        rect.adjust(-MarkWidth2 + viewport->x(), -MarkWidth2, MarkWidth2 + viewport->x(), MarkWidth2);
        m_markRect->setGeometry(rect);
    }
}

void TextView::updateQuickActionRect()
{
    if (m_quickActionButton->isHidden())
        return;

    auto *textEdit = m_document->textEdit();
    const int top = textEdit->cursorRect().translated(textEdit->viewport()->pos()).top();
    m_quickActionButton->move(textEdit->viewport()->x() - m_quickActionButton->width(), top);
}

void TextView::showQuickActionMenu()
{
    QMenu menu;
    menu.setToolTipsVisible(true);
    // TODO: Add actions
    menu.exec(QCursor::pos());
}

} // namespace Gui
