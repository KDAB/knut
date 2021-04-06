#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QSharedPointer>
#include <QString>

#include <spdlog/spdlog.h>

#include <functional>

namespace Script {

/**
 * \brief Script Runner
 *
 * Provide a script engine and a way to run scripts.
 * The script engine is initialized with interfaces.
 */
class ScriptRunner : public QObject
{
    Q_OBJECT

public:
    explicit ScriptRunner(QObject *parent = nullptr);
    ~ScriptRunner();

    // Run a script
    using EndScriptFunc = std::function<void()>;
    QVariant runScript(const QString &filePath, EndScriptFunc endCallback = {});

    bool hasError() const { return m_hasError; }
    QList<QQmlError> errors() const { return m_errors; }

private:
    QQmlEngine *getEngine(const QString &fileName);
    QVariant runJavascript(const QString &fileName, QQmlEngine *engine);
    QVariant runQml(const QString &fileName, QQmlEngine *engine);
    void filterErrors(const QQmlComponent &component);

private:
    std::shared_ptr<spdlog::logger> m_logger;
    bool m_hasError = false;
    QList<QQmlError> m_errors;
};

}
