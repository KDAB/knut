/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QRegularExpression>
#include <QString>

namespace Core {

enum class Case {
    CamelCase,
    PascalCase,
    SnakeCase,
    UpperCase,
    KebabCase,
    TitleCase,
};
QString convertCase(const QString &str, Case from, Case to);

QString matchCaseReplacement(const QString &originalText, const QString &replaceText);
QString expandRegExpReplacement(const QString &replaceText, const QStringList &capturedTexts);

/**
 * @brief createRegularExpression
 * Create a regular expression based on options from TextDocument::FindFlags
 */
QRegularExpression createRegularExpression(const QString &txt, int flags, bool isRegExp = true);

} // namespace Core
