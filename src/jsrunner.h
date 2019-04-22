#ifndef JSRUNNER_H
#define JSRUNNER_H

#include <QList>
#include <QObject>
#include <QQmlError>

class QQmlEngine;

class JsRunner : public QObject
{
    Q_OBJECT
public:
    explicit JsRunner(QObject *parent = nullptr);

    void setContextProperty(const QString &name, QObject *object);
    void setContextProperty(const QString &name, const QVariant &value);

    QVariant runJavaScript(const QString &scriptName);

    QList<QQmlError> errors() const;

protected:
    QQmlEngine *m_engine;
    QList<QQmlError> m_errors;
};

#endif // JSRUNNER_H
