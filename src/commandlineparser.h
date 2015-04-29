#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QStringList>
#include <QString>

enum Action
{
    AssetPath,
    Dialog,
    String,
    Default
};

enum ReturnCode
{
    MissingInputFile,
    InputFileNotFound,
    Success
};

struct Arguments
{
    QStringList positionalArguments;
    QString inputFile;
    QString resourceFile;
    QString asset;
    QString ui;
    QString string;
    QString outputFile;

    bool createQrc;

    Action action;
};

class QCommandLineParser;

int parseArguments(QCommandLineParser &parser, Arguments *args);

#endif // COMMANDLINEPARSER_H
