#ifndef JSRUNNER_H
#define JSRUNNER_H

#include <QList>
#include <QObject>
#include <QQmlError>

class QQmlEngine;

class JsResult : public QObject
{
    Q_OBJECT

public:
    struct Document
    {
        QString title;
        QString content;
    };

public:
    JsResult(QObject *parent = nullptr);

    Q_INVOKABLE QVariant createDocument(const QString &title);
    QVector<Document> documents() const;

private:
    QVector<Document> m_documents;
};

class JsResultDocumentWrapper
{
    Q_GADGET
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString content READ content WRITE setContent)

public:
    JsResultDocumentWrapper(JsResult::Document *document = nullptr);

    QString title() const;
    void setTitle(const QString &text);

    QString content() const;
    void setContent(const QString &text);

private:
    JsResult::Document *m_document = nullptr;
};
Q_DECLARE_METATYPE(JsResultDocumentWrapper);

class JsRunner : public QObject
{
    Q_OBJECT
public:
    explicit JsRunner(QObject *parent = nullptr);

    void setContextProperty(const QString &name, QObject *object);
    void setContextProperty(const QString &name, const QVariant &value);

    QVector<JsResult::Document> runJavaScript(const QString &scriptName);

    QList<QQmlError> errors() const;

protected:
    QQmlEngine *m_engine;
    QList<QQmlError> m_errors;
};

#endif // JSRUNNER_H
