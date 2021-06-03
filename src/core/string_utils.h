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

} // namespace Core
