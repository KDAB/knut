#include "document.h"

#include "rclexer.h"
#include "rcparser.h"

#include <QLoggingCategory>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>


extern QJsonArray yydialogs;

Q_LOGGING_CATEGORY(parser, "parser")
int linenum = 1;

namespace {
QString rcFile;
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

        m_dialogs = QJsonDocument(yydialogs);
        qDebug() << m_dialogs.toJson();
    } else {
        qCCritical(parser) << "Can't read " << fileName;
    }
}

