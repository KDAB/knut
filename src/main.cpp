#include "commandlineparser.h"
#include "documentloader.h"
#include "document.h"
#include "xmlwriter.h"

#include <QDebug>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QPair>
#include <QFileInfo>

#include <cstdlib>

using Result = QPair<QString, QByteArray>;
using ResultList = QList<Result>;

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

static void writeResult(const QByteArray &result, const QString &filename)
{
    QFile file(filename);

    if (file.open(QIODevice::WriteOnly))
        file.write(result);
    else
        qCritical() << QObject::tr("can't write file %1").arg(filename);
}

static void writeResults(const ResultList &results)
{
    for (const Result &r : results)
        writeResult(r.second, r.first);
}

static Result handleDialog(const QJsonObject &dialog)
{
    Result result;

    result.first = dialogId(dialog) + ".ui";
    result.second = dialogToUi(dialog);

    return result;
}

static Result handleDialog(const QJsonObject &root, const QString &id,
        const QString &filename = QString())
{
    const auto dialog = documentDialog(root, id);

    Result r = handleDialog(dialog);

    if (!filename.isEmpty())
        r.first = filename;

    return r;
}

static ResultList handleDefault(const QJsonObject &o)
{
    QJsonObject dialogs = documentDialogs(o);

    ResultList l;

    Q_FOREACH (const QJsonValue &value, dialogs) {
        const auto dialog = value.toObject();

        Result r = handleDialog(dialog);
        l.append(r);
    }

    return l;
}

static ResultList handleQrcFile(const QJsonObject &o, const QString &outputFile)
{
    QJsonObject assets = documentAssets(o);

    Result result;

    result.first = outputFile;
    result.second = assetsToQrc(assets);

    return ResultList() << result;
}

static QTextStream &qStdOut()
{
    static QTextStream s(stdout);
    return s;
}

static void handleAsset(const QJsonObject &o, const QString &assetId)
{
    qStdOut() << documentAsset(o, assetId) << "\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("knut");
    QCoreApplication::setApplicationVersion("1.0");

    Arguments args;

    const auto action = parseArgs(&args);

    QJsonObject rootObject = loadDocument(args.inputFile, args.resourceFile);

    ResultList results;

    switch (action) {
    case AssetPath:
        handleAsset(rootObject, args.asset);
        break;
    case Dialog:
        results << handleDialog(rootObject, args.ui);
        break;
    case String:
        //TODO
        break;
    case QrcFile:
        results << handleQrcFile(rootObject,
                QFileInfo(args.inputFile).baseName() + ".qrc");
        break;
    case Default:
        results << handleDefault(rootObject);
        break;
    }

    writeResults(results);

    return 0;
}
