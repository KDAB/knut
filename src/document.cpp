#include "document.h"

#include "rclexer.h"
#include "rcparser.h"

#include <QLoggingCategory>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QJsonDocument>


extern QJsonObject yydata;

Q_LOGGING_CATEGORY(parser, "parser")
Q_LOGGING_CATEGORY(data, "data")
int linenum = 1;

namespace {
QString rcFile;
const QString AssetsKey = QStringLiteral("assets");
const QString DialogsKey = QStringLiteral("dialogs");
}

void yyerror(const char *s)
{
    qCCritical(parser, "%s:%d: %s\n", rcFile.toLatin1().constData(), linenum, s);
}


Document::Document(const QString &fileName)
{
    QFileInfo fi(fileName);
    rcFile = fi.fileName();
    parse(fileName);
}

Document::~Document()
{

}

QString Document::getAsset(const QString &id)
{
    auto value = m_data.value(AssetsKey).toObject().value(id);
    if (value.isUndefined())
        qCCritical(data, "Asset %s does not exist\n", id);
    return value.toString();
}

QJsonObject Document::getDialog(const QString &id)
{
    auto value = m_data.value(DialogsKey).toObject().value(id);
    if (value.isUndefined())
        qCCritical(data, "Dialog %s does not exist\n", id);
    return value.toObject();
}

void Document::parse(const QString &fileName)
{
    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);

        // Bison take the standard input (command line) as input.
        // yy_scan_string will switch the input to a string.
        YY_BUFFER_STATE bufferState = yy_scan_string(stream.readAll().toUtf8().constData());

        // Parse the string.
        yyparse();

        // and release the buffer.
        yy_delete_buffer(bufferState);

        m_data = yydata;
        qDebug() << QJsonDocument(m_data).toJson();
    } else {
        qCCritical(parser) << "Can't read " << fileName;
    }
}

