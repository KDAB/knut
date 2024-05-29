/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "logpanel.h"
#include "guisettings.h"
#include "utils/log.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMetaObject>
#include <QPlainTextEdit>
#include <QPointer>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QToolButton>
#include <mutex>
#include <spdlog/sinks/qt_sinks.h>

namespace Gui {

class LogHighlighter : public QSyntaxHighlighter
{
public:
    explicit LogHighlighter(QTextDocument *parent = nullptr)
        : QSyntaxHighlighter(parent)
    {
        auto getFormat = [](const QColor &color, const QColor &background = {}) {
            QTextCharFormat format;
            format.setForeground(color);
            if (background.isValid())
                format.setBackground(background);
            return format;
        };
        m_rules = {
            {"[trace]", getFormat(QColor(128, 128, 128))},
            {"[debug]", getFormat(Qt::cyan)},
            {"[info]", getFormat(Qt::green)},
            {"[warning]", getFormat(QColor(255, 220, 0))},
            {"[error]", getFormat(Qt::red)},
            {"[critical]", getFormat(Qt::white, Qt::red)},
        };
    }

protected:
    void highlightBlock(const QString &text) override
    {
        for (const auto &rule : std::as_const(m_rules)) {
            if (int start = text.indexOf(rule.keyword); start != -1)
                setFormat(start + 1, rule.keyword.length() - 2, rule.format);
        }
    }

private:
    struct HighlightingRule
    {
        QString keyword;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> m_rules;
};

LogPanel::LogPanel(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_toolBar(new QWidget)
{
    setWindowTitle(tr("Log Output"));
    setObjectName("LogPanel");
    setReadOnly(true);

    auto logger = spdlog::default_logger();
    logger->sinks().push_back(std::make_shared<spdlog::sinks::qt_sink_mt>(this, "appendPlainText"));

    // Setup text edit
    new LogHighlighter(document());
    setWordWrapMode(QTextOption::NoWrap);
    GuiSettings::setupTextEdit(this);

    // Setup titlebar
    auto layout = new QHBoxLayout(m_toolBar);
    layout->setContentsMargins({});

    auto clearButton = new QToolButton(m_toolBar);
    GuiSettings::setIcon(clearButton, ":/gui/delete-sweep.png");
    clearButton->setToolTip(tr("Clear"));
    clearButton->setAutoRaise(true);
    layout->addWidget(clearButton);
    connect(clearButton, &QToolButton::clicked, this, &QPlainTextEdit::clear);

    layout->addWidget(new QLabel(tr("Level:")));
    auto levelCombo = new QComboBox(m_toolBar);
    levelCombo->addItems({"trace", "debug", "info", "warning", "error", "critical"});
    levelCombo->setCurrentIndex(logger->level());
    layout->addWidget(levelCombo);
    connect(levelCombo, qOverload<int>(&QComboBox::currentIndexChanged), levelCombo, [logger](int index) {
        logger->set_level(static_cast<spdlog::level::level_enum>(index));
    });
}
LogPanel::~LogPanel()
{
    auto logger = spdlog::default_logger();
    logger->sinks().pop_back();
}

QWidget *LogPanel::toolBar() const
{
    return m_toolBar;
}

} // namespace Gui
