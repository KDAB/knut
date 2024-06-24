/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QSet>
#include <QSharedPointer>
#include <QString>
#include <functional>

namespace Core {

/**
 * \brief Runner for scripts
 *
 * Provide a script engine and a way to run scripts.
 * The script engine is initialized with interfaces.
 */
class ScriptRunner : public QObject
{
    Q_OBJECT

public:
    explicit ScriptRunner(QObject *parent = nullptr);
    ~ScriptRunner() override;

    // Run a script
    using EndScriptFunc = std::function<void()>;
    QVariant runScript(const QString &fileName, const EndScriptFunc &endCallback = {});

    bool hasError() const { return m_hasError; }
    QList<QQmlError> errors() const { return m_errors; }

    static bool isProperty(const QString &apiCall);

private:
    QQmlEngine *getEngine(const QString &fileName);
    QVariant runJavascript(const QString &fileName, QQmlEngine *engine);
    QVariant runQml(const QString &fileName, QQmlEngine *engine);
    void filterErrors(const QQmlComponent &component);

private:
    friend class ScriptDialogItem;
    inline static QString currentScriptPath;

    bool m_hasError = false;
    QList<QQmlError> m_errors;

    inline static QSet<QString> m_properties = {};
};

} // namespace Core
