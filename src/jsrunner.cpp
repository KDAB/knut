#include "jsrunner.h"

#include <QFileInfo>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QTemporaryFile>
#include <QTextStream>

JsRunner::JsRunner(QObject *parent)
    : QObject(parent)
    , m_engine(new QQmlEngine(this))
{
}

void JsRunner::setContextProperty(const QString &name, QObject *object)
{
    m_engine->rootContext()->setContextProperty(name, object);
}

void JsRunner::setContextProperty(const QString &name, const QVariant &value)
{
    m_engine->rootContext()->setContextProperty(name, value);
}

QVariant JsRunner::runJavaScript(const QString &scriptName)
{
    const QUrl fileUrl = QUrl::fromLocalFile(QFileInfo(scriptName).absoluteFilePath());
    const QString wrapperQml =
        QString(QLatin1String("import QtQml 2.2\n"
                              "import \"%1\" as Script\n"
                              "QtObject { function run() { return Script.main(); } }"))
            .arg(fileUrl.toString());

    QQmlComponent component(m_engine);
    component.setData(wrapperQml.toUtf8(), fileUrl);

    QObject *scriptObject = component.create();
    m_errors = component.errors();
    if (!component.isReady() || component.isError())
        return QVariant();

    QVariant result;
    QMetaObject::invokeMethod(scriptObject, "run", Q_RETURN_ARG(QVariant, result));
    return result;
}

QList<QQmlError> JsRunner::errors() const
{
    return m_errors;
}
