#include "string_utils.h"

#include <QSet>

namespace Core {

static QString nextWordInString(const QString &str, QString::const_iterator &i, Case c)
{
    QString word;

    auto isSeparator = [&i, c]() {
        if (c == Case::CamelCase || c == Case::PascalCase)
            return i->isUpper();
        else if (c == Case::SnakeCase || c == Case::UpperCase)
            return *i == QLatin1Char('_');
        else if (c == Case::KebabCase)
            return *i == QLatin1Char('-');
        return *i == QLatin1Char(' ');
    };

    do {
        word += *i;
        i++;
    } while (!isSeparator() && i != str.end());

    if (i != str.end()
        && (c == Case::SnakeCase || c == Case::KebabCase || c == Case::UpperCase || c == Case::TitleCase))
        i++;

    return word;
}

QString convertCase(const QString &str, Case from, Case to)
{
    if (from == to)
        return str;

    static const QSet<QString> titleCaseExceptions = {"a",  "an", "the", "at", "by",  "by", "for",
                                                      "in", "of", "on",  "to", "and", "as", "or"};

    QString result;

    auto i = str.cbegin();
    while (i < str.cend()) {
        QString w = nextWordInString(str, i, from);
        const bool firstWord = result.isEmpty();

        switch (to) {
        case Case::CamelCase:
            w = w.toLower();
            if (!firstWord)
                w[0] = w.front().toUpper();
            result += w;
            break;
        case Case::PascalCase:
            w = w.toLower();
            w[0] = w.front().toUpper();
            result += w;
            break;
        case Case::SnakeCase:
            if (!firstWord)
                result += QLatin1Char('_');
            result += w.toLower();
            break;
        case Case::KebabCase:
            if (!firstWord)
                result += QLatin1Char('-');
            result += w.toLower();
            break;
        case Case::UpperCase:
            if (!firstWord)
                result += QLatin1Char('_');
            result += w.toUpper();
            break;
        case Case::TitleCase:
            if (!firstWord)
                result += QLatin1Char(' ');
            w = w.toLower();
            if (firstWord || !titleCaseExceptions.contains(w))
                w[0] = w.front().toUpper();
            result += w;
        }
    }

    return result;
}

} // namespace Migration
