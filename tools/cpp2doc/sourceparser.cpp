#include "sourceparser.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

SourceParser::SourceParser(Data &data)
    : m_data(data)
{
}

static QStringList getAllSourceFiles(const QString &directory)
{
    QStringList result;

    QDir dir(directory);
    auto files = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

    for (const auto &fileInfo : files) {
        if (fileInfo.suffix() == "cpp" || fileInfo.suffix() == "qml")
            result += fileInfo.absoluteFilePath();
        if (fileInfo.isDir())
            result += getAllSourceFiles(fileInfo.absoluteFilePath());
    }
    return result;
}

void SourceParser::parseDirectory(const QString &directory)
{
    QStringList sourceFiles = getAllSourceFiles(directory);
    for (const auto &sourceFile : sourceFiles)
        parseFile(sourceFile);
}

QString SourceParser::cleanupCommentLine(QString line)
{
    if (line.startsWith(" *") || line.startsWith("   "))
        line = line.mid(3);
    return line;
}

void SourceParser::parseFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Can't read file: " << fileName;
        return;
    }

    QTextStream stream(&file);

    bool inDocumentation = false;
    while (!stream.atEnd()) {
        QString line = stream.readLine();

        if (line.isEmpty())
            continue;
        if (line.startsWith("/*!"))
            inDocumentation = true;
        if (inDocumentation) {
            line = cleanupCommentLine(line);
            if (line.startsWith("\\qmltype"))
                m_data.types.push_back(parseType(stream, line));
            else if (line.startsWith("\\qmlproperty"))
                m_data.properties.push_back(parseProperty(stream, line));
            else if (line.startsWith("\\qmlmethod"))
                m_data.methods.push_back(parseMethod(stream, line));
            else if (line.startsWith("\\qmlsignal"))
                m_data.qmlSignals.push_back(parseSignal(stream, line));
        }
    }
}

Data::TypeBlock SourceParser::parseType(QTextStream &stream, QString line)
{
    Data::TypeBlock currentType;
    currentType.name = line.mid(9);

    while (!stream.atEnd()) {
        line = stream.readLine();

        if (line.startsWith(" */"))
            break;
        line = cleanupCommentLine(line);

        if (line.startsWith("\\instantiates"))
            continue;
        else if (line.startsWith("\\inqmlmodule"))
            currentType.qmlModule = line.mid(13);
        else if (line.startsWith("\\inherits"))
            currentType.inherits = line.mid(10);
        else
            parseBlock(line, currentType);
    }

    return currentType;
}

Data::PropertyBlock SourceParser::parseProperty(QTextStream &stream, QString line)
{
    Data::PropertyBlock currentProperty;
    line = line.mid(13);
    currentProperty.name = line;
    QRegularExpression regexp(R"(^([\w<>]+) (\w+)::(\w+)$)");
    auto match = regexp.match(line);
    currentProperty.type = match.captured(1);
    currentProperty.qmlType = match.captured(2);
    currentProperty.name = match.captured(3);

    while (!stream.atEnd()) {
        line = stream.readLine();

        if (line.startsWith(" */"))
            break;
        line = cleanupCommentLine(line);

        parseBlock(line, currentProperty);
    }

    return currentProperty;
}

static auto parseMethodDefinition(const QString &line)
{
    struct Result
    {
        Data::QmlMethod method;
        QString qmlType;
    } result;

    QRegularExpression regexp(R"(^(?:([\w<>]+) )?(\w+)::(\w+)\((.*)\)$)");
    auto match = regexp.match(line);
    result.method.returnType = match.captured(1);
    result.qmlType = match.captured(2);
    result.method.name = match.captured(3);

    QStringList parameters = match.captured(4).split(',');
    for (auto text : parameters) {
        Data::QmlMethod::Parameter param;
        text = text.simplified();
        int space = text.lastIndexOf(' ');
        if (space != -1) {
            param.type = text.left(space);
            param.name = text.mid(space + 1);
        } else {
            param.name = text;
        }
        result.method.parameters.push_back(param);
    }

    return result;
}

Data::MethodBlock SourceParser::parseMethod(QTextStream &stream, QString line)
{
    Data::MethodBlock currentMethod;
    line = line.mid(11);

    auto definition = parseMethodDefinition(line);
    currentMethod.qmlType = definition.qmlType;
    currentMethod.methods.push_back(definition.method);

    while (!stream.atEnd()) {
        line = stream.readLine();

        if (line.startsWith(" */"))
            break;
        line = cleanupCommentLine(line);

        if (line.startsWith("\\qmlmethod")) {
            auto definition = parseMethodDefinition(line.mid(11));
            currentMethod.methods.push_back(definition.method);
        } else {
            parseBlock(line, currentMethod);
        }
    }

    return currentMethod;
}

Data::SignalBlock SourceParser::parseSignal(QTextStream &stream, QString line)
{
    Data::SignalBlock currentSignal;
    line = line.mid(11);

    auto definition = parseMethodDefinition(line);
    currentSignal.qmlType = definition.qmlType;
    currentSignal.method = definition.method;

    while (!stream.atEnd()) {
        line = stream.readLine();

        if (line.startsWith(" */"))
            break;
        line = cleanupCommentLine(line);

        parseBlock(line, currentSignal);
    }

    return currentSignal;
}

void SourceParser::parseBlock(const QString &line, Data::Block &block)
{
    if (line.startsWith("\\brief"))
        block.brief = line.mid(7);
    else if (line.startsWith("\\since"))
        block.since = line.mid(7);
    else if (line.startsWith("\\sa"))
        block.seeAlso.push_back(line.mid(4));
    else if (line.startsWith("\\"))
        return; // Tag not handled
    else if (line.isEmpty() && block.description.isEmpty())
        return;
    else
        block.description += line + "\n";
}
