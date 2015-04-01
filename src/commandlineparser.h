#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QStringList>
#include <QString>

struct Arguments
{
    QStringList positionalArguments;
    QString inputFile;
    QString resourceFile;
    QString asset;
    QString ui;
    QString string;
    QString outputFile;
    QString qrcFile;
    bool hasQrc;
};

enum
{
    MissingInputFile,
    InputFileNotFound,
    AssetPath,
    Dialog,
    String,
    QrcFile,
    Default
};

class QCommandLineParser;

int parseArguments(QCommandLineParser &parser, Arguments *args);

#endif // COMMANDLINEPARSER_H
