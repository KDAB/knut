#include "jsrunner.h"

#include "logging.h"

#include <QFileInfo>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QTemporaryFile>
#include <QTextStream>

//=============================================================================
// JsResult
//=============================================================================
JsResult::JsResult(QObject *parent)
    : QObject(parent)
{
}

QVariant JsResult::createDocument(const QString &title)
{
    m_documents.push_back({title, ""});
    JsResultDocumentWrapper doc(&m_documents.last());
    return QVariant::fromValue(doc);
}

QVector<JsResult::Document> JsResult::documents() const
{
    return m_documents;
}

JsRunner::JsRunner(QObject *parent)
    : QObject(parent)
    , m_engine(new QQmlEngine(this))
{
}

JsResultDocumentWrapper::JsResultDocumentWrapper(JsResult::Document *document)
    : m_document(document)
{
}

QString JsResultDocumentWrapper::title() const
{
    if (m_document)
        return m_document->title;
    return {};
}

void JsResultDocumentWrapper::setTitle(const QString &text)
{
    if (m_document)
        m_document->title = text;
}

QString JsResultDocumentWrapper::content() const
{
    if (m_document)
        return m_document->content;
    return {};
}

void JsResultDocumentWrapper::setContent(const QString &text)
{
    if (m_document)
        m_document->content = text;
}

//=============================================================================
// JsRunner
//=============================================================================
void JsRunner::setContextProperty(const QString &name, QObject *object)
{
    m_engine->rootContext()->setContextProperty(name, object);
}

void JsRunner::setContextProperty(const QString &name, const QVariant &value)
{
    m_engine->rootContext()->setContextProperty(name, value);
}

QVector<JsResult::Document> JsRunner::runJavaScript(const QString &scriptName)
{
    const QUrl fileUrl = QUrl::fromLocalFile(QFileInfo(scriptName).absoluteFilePath());
    const QString wrapperQml =
        QString(QLatin1String("import QtQml 2.2\n"
                              "import \"%1\" as Script\n"
                              "QtObject { function run() { return Script.main(); } }"))
            .arg(fileUrl.toString());

    JsResult result;
    m_engine->rootContext()->setContextProperty("result", &result);

    QQmlComponent component(m_engine);
    component.setData(wrapperQml.toUtf8(), fileUrl);

    QObject *scriptObject = component.create();
    m_errors = component.errors();
    if (!component.isReady() || component.isError()) {
        qCWarning(JSRUNNER) << m_errors;
        return {};
    }

    QMetaObject::invokeMethod(scriptObject, "run");
    return result.documents();
}

QList<QQmlError> JsRunner::errors() const
{
    return m_errors;
}
