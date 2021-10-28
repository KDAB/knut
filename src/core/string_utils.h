#pragma once

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

} // namespace Core
