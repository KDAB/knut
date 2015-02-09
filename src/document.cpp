#include "document.h"

#include "rclexer.h"
#include "rcparser.h"

#include <QLoggingCategory>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>


extern QJsonObject yydata;

Q_LOGGING_CATEGORY(reader, "reader")
Q_LOGGING_CATEGORY(writer, "writer")
int linenum = 1;

namespace {
QString RcFile;
bool Error = false;
static const auto AssetsKey = QStringLiteral("assets");
static const auto DialogsKey = QStringLiteral("dialogs");
}


Document::Document(const QJsonObject &data)
{
    dialogs = data.value(DialogsKey).toObject().toVariantMap();
    assets = data.value(AssetsKey).toObject().toVariantMap();
    hasError = false;
}

QJsonObject Document::dialog(const QString &id)
{
    return dialogs.value(id).toJsonObject();
}

QString Document::assetPath(const QString &id)
{
    return assets.value(id).toString();
}


void yyerror(const char *s)
{
    qCCritical(reader, "%s:%d: %s\n", RcFile.toLatin1().constData(), linenum, s);
    Error = true;
}


Document readFromRcFile(const QString &rcFile,
                        const QString &resourceFile)
{
    RcFile = rcFile;
    QFile file(rcFile);

    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);

        // Bison take the standard input (command line) as input.
        // yy_scan_string will switch the input to a string.
        YY_BUFFER_STATE bufferState = yy_scan_string(stream.readAll().toUtf8().constData());

        // Parse the string.
        yyparse();

        // and release the buffer.
        yy_delete_buffer(bufferState);

        // No error, return the document
        if (!Error)
            return Document(yydata);
    }

    qCCritical(reader) << "Can't read " << rcFile;
    return Document();
}

Document readFomrJsonFile(const QString &jsonFile)
{
    QFile file(jsonFile);

    if (file.open(QIODevice::ReadOnly)) {
        auto jsonDoc = QJsonDocument::fromBinaryData(file.readAll());

        if (!jsonDoc.isNull())
            return Document(jsonDoc.object());
    }

    qCCritical(reader) << "Can't read " << jsonFile;
    return Document();
}


QJsonDocument createJsonDocument(const Document &doc)
{
    QJsonObject obj;
    obj.insert(AssetsKey, QJsonObject::fromVariantMap(doc.assets));
    obj.insert(DialogsKey, QJsonObject::fromVariantMap(doc.dialogs));

    return QJsonDocument(obj);
}

void writeToJsonFile(const Document &doc, const QString &jsonFile)
{
    QFile file(jsonFile);

    if (file.open(QIODevice::WriteOnly)) {
        auto jsonDoc = createJsonDocument(doc);
        file.write(jsonDoc.toJson());
    }
    qCCritical(writer) << "Can't write " << jsonFile;
}
