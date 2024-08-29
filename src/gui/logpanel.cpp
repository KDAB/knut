/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "logpanel.h"
#include "core/loghighlighter.h"
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
    new Core::LogHighlighter(document());
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
