/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "qt_fmt_format.h"

#include <QRegularExpression>
#include <source_location>
#include <spdlog/spdlog.h>

/**
 * Format the std::source_location::current().function_name() return value
 * to a simplified 'className::functionName' string value.
 * The FUNCTION_NAME macro is meant to be called by spdlog to point to the function location.
 * e.g: spdlog::error("{}: {} - cannot convert", FUNCTION_NAME, path);
 */

#define FUNCTION_NAME Core::formatToClassNameFunctionName(std::source_location::current())

namespace Core {

inline QString formatToClassNameFunctionName(const std::source_location &location)
{
    // Get the functionName.
    // str = e.g: "QVariant Core::Settings::value(QString, const QVariant&) const"
    QString str(location.function_name());

    // Extract the section before the arguments list.
    int openParenthesisIndex = str.indexOf("(");
    str = str.mid(0, openParenthesisIndex);
    // Split (whitespace and double columns).
    static const QRegularExpression splitRegexp("\\s+|::");
    QStringList sections = str.split(splitRegexp);

    // Assert in case we have less than 2 elements.
    Q_ASSERT(sections.size() >= 2);

    // Keep the 2 last elements of the list, remove the leading symbols.
    sections = sections.mid(sections.size() - 2);
    static const QRegularExpression symbolsRegexp("[*&]");
    str = sections[0].remove(symbolsRegexp) + "::" + sections[1];
    return str;
}

} // namespace Core
