#include "commandlineparser.h"

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QCoreApplication>
#include <QFileInfo>

int parseArguments(QCommandLineParser &parser, Arguments *args)
{
    Q_ASSERT(args);

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

    QCommandLineOption qrcOption("qrc", "Create qrc file");
    parser.addOption(qrcOption);

    QCommandLineOption outputOption(QStringList() << "o" << "output",
                                    "Output file",
                                    "file");
    parser.addOption(outputOption);

    // Process the actual command line arguments given by the user
    parser.process(QCoreApplication::arguments());

    args->positionalArguments = parser.positionalArguments();

    const auto arguments = parser.positionalArguments();


    if (arguments.isEmpty())
        return MissingInputFile;

    args->positionalArguments = arguments;

    const QString input = arguments.first();

    QFileInfo fi(input);
    if (!fi.exists())
        return InputFileNotFound;

    args->inputFile = input;

    // Get resource file
    QString resource = parser.value(resourceOption);
    if (resource.isEmpty()) {
        if (QFileInfo::exists(fi.canonicalPath() + "/Resource.h"))
            resource = fi.canonicalPath() + "/Resource.h";
        if (QFileInfo::exists(fi.canonicalPath() + "/resource.h"))
            resource = fi.canonicalPath() + "/resource.h";
    }

    args->resourceFile = resource;

    args->asset = parser.value(assetOption);
    args->ui = parser.value(uiOption);
    args->string = parser.value(stringOption);
    args->outputFile = parser.value(outputOption);
    args->createQrc = parser.isSet(qrcOption);

    if (!args->asset.isEmpty())
        return AssetPath;
    else if (!args->ui.isEmpty())
        return Dialog;
    else if (!args->string.isEmpty())
        return String;
    else if (args->createQrc)
        return QrcFile;

    return Default;
}
