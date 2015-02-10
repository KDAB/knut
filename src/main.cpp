#include "document.h"

#include <QDebug>
#include <QJsonDocument>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFileInfo>
#include <QFile>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("knut");
    QCoreApplication::setApplicationVersion("1.0");

    // Create command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Rc to Qt converter");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("input", "Input file, either rc file or json file");

    QCommandLineOption resourceOption("resource", "Define resource header",
                                      "file");
    parser.addOption(resourceOption);

    QCommandLineOption assetOption("asset", "Return the asset path for <id>", "id");
    parser.addOption(assetOption);

    QCommandLineOption uiOption("ui", "Create ui file for <id>", "id");
    parser.addOption(uiOption);

    QCommandLineOption stringOption("string", "Return the string for <id>", "id");
    parser.addOption(stringOption);

    QCommandLineOption qrcOption("qrc", "Create or use qrc file");
    parser.addOption(qrcOption);

    QCommandLineOption outputOption(QStringList() << "o" << "output",
                                    "Output file",
                                    "file");
    parser.addOption(outputOption);

    // Process the actual command line arguments given by the user
    parser.process(app);

    // Get the rc/json file
    const QString input = parser.positionalArguments().first();
    QFileInfo fi(input);
    if (!fi.exists()) {
        qCritical("Input file %s not found", input);
        return -1;
    }
    bool useJson = fi.completeSuffix() == "json";

    // Get resource file
    QString resource = parser.value(resourceOption);
    if (resource.isEmpty()) {
        if (QFileInfo::exists(fi.canonicalPath() + "/Resource.h"))
            resource = fi.canonicalPath() + "/Resource.h";
        if (QFileInfo::exists(fi.canonicalPath() + "/resource.h"))
            resource = fi.canonicalPath() + "/resource.h";
    }

    Document doc;
    if (useJson)
        doc = readFromJsonFile(input);
    else
        doc = readFromRcFile(input, resource);

    const QString asset = parser.value(assetOption);
    const QString ui = parser.value(uiOption);
    const QString string = parser.value(stringOption);
    const bool hasQrc = parser.isSet(qrcOption);

    const QString output = parser.value(outputOption);

    // Do the actual work
    QByteArray result;

    if (!asset.isEmpty()) {
        // TODO: return asset path
    } else if (!ui.isEmpty()) {
        // TODO: return only one dialog
    } else if (!string.isEmpty()) {
        // TODO: return string
    } else if (hasQrc) {
        // TODO: save qrc file
    } else {
        result = createJsonDocument(doc).toJson();
    }

    if (output.isEmpty()) {
        qDebug() << result;
    } else {
        QFile file(output);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(result);
        } else {
            qCritical("can't write file %s", output);
            return -1;
        }
    }

    return 0;
}
