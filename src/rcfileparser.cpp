#include "rclexer.h"
#include "rcparser.h"
#include "rcfileparser.h"

#include <QFile>
#include <QString>
#include <QByteArray>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(rcparser, "rcparser")

extern QJsonObject yydata;

static const char *filename = 0;
static bool error = false;

extern int yylinenum;

void yyerror(const char *s)
{
    qCCritical(rcparser, "%s:%d: %s\n", filename, yylinenum, s);
    error = true;
}

QJsonObject parseRcFile(const QString &rcFile)
{
    const QByteArray buffer = rcFile.toLatin1();
    filename = buffer.constData();

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
        if (!error)
            return yydata;
    }

    qCCritical(rcparser) << "Can't read " << rcFile;
    return QJsonObject();
}
