/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "utils/strings.h"

#include <QHash>
#include <QObject>

namespace Core {

class Utils : public QObject
{
    Q_OBJECT

public:
    enum Case {
        CamelCase = static_cast<int>(::Core::Case::CamelCase),
        PascalCase = static_cast<int>(::Core::Case::PascalCase),
        SnakeCase = static_cast<int>(::Core::Case::SnakeCase),
        UpperCase = static_cast<int>(::Core::Case::UpperCase),
        KebabCase = static_cast<int>(::Core::Case::KebabCase),
        TitleCase = static_cast<int>(::Core::Case::TitleCase),
    };
    Q_ENUM(Case)

    explicit Utils(QObject *parent = nullptr);
    ~Utils() override;

public slots:
    static QString getEnv(const QString &varName);

    static QString getGlobal(const QString &varName);
    static void setGlobal(const QString &varName, const QString &value);

    static void addScriptPath(const QString &path);
    static void runScript(const QString &path, bool log = false);

    static void sleep(int msecs);

    static QString mktemp(const QString &pattern);

    static QString convertCase(const QString &str, Core::Utils::Case from, Core::Utils::Case to);

    static void copyToClipboard(const QString &text);

    static QStringList cppKeywords();

    static QStringList cppPrimitiveTypes();

private:
    static QHash<QString, QString> m_globals;
};

} // namespace Core
