#include "commandlineparser.h"
#include "documentloader.h"
#include "document.h"

#include <QDebug>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFile>

#include <cstdlib>

static int parseArgs(Arguments *args)
{
    QCommandLineParser parser;

    const auto ret = parseArguments(parser, args);

    switch (ret) {
    case MissingInputFile:
        qCritical() << QObject::tr("Error: Missing input file.");
        parser.showHelp(EXIT_FAILURE);
    case InputFileNotFound:
        qCritical()
            << QObject::tr("Input file %1 not found").arg(args->inputFile);
        exit(EXIT_FAILURE);
    default:
        break;
    }

    return ret;
}

static void writeResult(const QByteArray &result,
        const QString &filename = QString())
{
    // TODO: if filename is empty then set it to resource id
    if (filename.isEmpty()) {
        qDebug() << result;
        return;
    }

    QFile file(filename);

    if (file.open(QIODevice::WriteOnly))
        file.write(result);
    else
        qCritical() << QObject::tr("can't write file %1").arg(filename);
}

static void writeResults(const QByteArrayList &results, const QString &filename)
{
    foreach (const QByteArray &b, results)
        writeResult(b, filename);
}

static inline QByteArray handleDialog(
        const QJsonObject &root, const QString &id)
{
    return documentToByteArray(documentDialog(root, id));
}

static inline QByteArrayList handleDefault(const QJsonObject &o)
{
    QByteArrayList l;
    l << documentToByteArray(documentDialogs(o));

    return l;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("knut");
    QCoreApplication::setApplicationVersion("1.0");

    Arguments args;

    const auto action = parseArgs(&args);

    QJsonObject rootObject = loadDocument(args.inputFile, args.resourceFile);

    QByteArrayList results;

    switch (action) {
    case AssetPath:
    case Dialog:
        results << handleDialog(rootObject, args.ui);
        break;
    case String:
        //TODO
        break;
    case QrcFile:
        //TODO
        break;
    case Default:
        results << handleDefault(rootObject);
        break;
    }

    writeResults(results, args.outputFile);

    return 0;
}
