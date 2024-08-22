/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "sourceparser.h"

#include <QDateTime>
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
    const auto files = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

    for (const auto &fileInfo : files) {
        if (fileInfo.suffix() == "cpp" || fileInfo.suffix() == "qml")
            result += fileInfo.absoluteFilePath();
        if (fileInfo.isDir())
            result += getAllSourceFiles(fileInfo.absoluteFilePath());
    }
    return result;
}

void SourceParser::loadMappingFile(const QString &filePath)
{
    QFile mappingFile(filePath + "/mapping.txt");
    if (!mappingFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Unable to open file mapping.txt for reading.";
        return;
    }

    QTextStream in(&mappingFile);

    while (!in.atEnd()) {
        const QString line = in.readLine();
        const QStringList parts = line.split(',');

        if (parts.size() == 6) {
            Data::MappedType entry;
            entry.typeName = parts[0];
            entry.sourceFile = parts[1];
            entry.docFile = parts[2];
            entry.qmlModule = parts[3];
            entry.group = parts[4];
            entry.positionInGroup = static_cast<Data::PositionInGroup>(parts[5].toInt());

            m_data.mappedTypes.push_back(std::move(entry));
        } else {
            qWarning() << "Invalid line format in mapping.txt: " << line;
        }
    }
}

void SourceParser::parseDirectory(const QString &directory)
{
    const QStringList sourceFiles = getAllSourceFiles(directory);

    for (const auto &sourceFile : sourceFiles) {
        const QFileInfo sourceFileInfo(sourceFile);
        const QDateTime sourceLastModified = sourceFileInfo.lastModified();

        const QString relativeSourceFile = QDir(KNUT_SOURCE_PATH).relativeFilePath(sourceFileInfo.absoluteFilePath());

        auto isMatchingSourceFile = [&relativeSourceFile](const Data::MappedType &entry) {
            return entry.sourceFile == relativeSourceFile;
        };

        auto it = std::ranges::find_if(m_data.mappedTypes, isMatchingSourceFile);

        if (it != m_data.mappedTypes.end()) {
            const QFileInfo docFileInfo(QDir(KNUT_DOC_PATH).absoluteFilePath(it->docFile));
            const QDateTime docLastModified = docFileInfo.lastModified();

            if (sourceLastModified > docLastModified) {
                std::erase_if(m_data.mappedTypes, isMatchingSourceFile);
                parseFile(sourceFile);
            }
        } else
            parseFile(sourceFile);
    }
    auto fileDoesNotExist = [](const Data::MappedType &entry) {
        return !QFileInfo::exists(QDir(KNUT_SOURCE_PATH).absoluteFilePath(entry.sourceFile));
    };

    std::erase_if(m_data.mappedTypes, fileDoesNotExist);
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
            if (line.startsWith("\\qmltype")) {
                const Data::TypeBlock currentType = parseType(stream, line);
                m_data.types.push_back(currentType);
                updateFileMap(fileName, currentType);
            } else if (line.startsWith("\\qmlproperty"))
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

        if (line.startsWith("\\inqmlmodule"))
            currentType.qmlModule = line.mid(13);
        else if (line.startsWith("\\inherits"))
            currentType.inherits = line.mid(10);
        else
            parseBlock(line, currentType);
    }

    if (currentType.qmlModule.isEmpty()) {
        currentType.qmlModule = "Knut";
    }

    return currentType;
}

void SourceParser::updateFileMap(const QString &fileName, const Data::TypeBlock &currentType)
{
    const QString relativeSourceFileName = QDir(KNUT_SOURCE_PATH).relativeFilePath(fileName);
    const QString relativeDocFileName =
        QString("API/%1/%2.md").arg(currentType.qmlModule.toLower(), currentType.name.toLower());

    auto isSameFile = [&currentType, &relativeSourceFileName](const Data::MappedType &entry) {
        return entry.typeName == currentType.name && entry.sourceFile == relativeSourceFileName;
    };
    auto it = std::ranges::find_if(m_data.mappedTypes, isSameFile);

    if (it == m_data.mappedTypes.end()) {
        Data::MappedType entry;
        entry.typeName = currentType.name;
        entry.sourceFile = relativeSourceFileName;
        entry.docFile = relativeDocFileName;
        entry.qmlModule = currentType.qmlModule;
        entry.group = currentType.group;
        entry.positionInGroup = currentType.positionInGroup;

        m_data.mappedTypes.push_back(std::move(entry));
    } else {
        it->qmlModule = currentType.qmlModule;
        it->group = currentType.group;
        it->positionInGroup = currentType.positionInGroup;
        it->docFile = relativeDocFileName;
    }
}

Data::PropertyBlock SourceParser::parseProperty(QTextStream &stream, QString line)
{
    Data::PropertyBlock currentProperty;
    line = line.mid(13);
    static QRegularExpression regexp(R"(^([\w<>]+) (\w+)::(\w+)$)");
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

    static QRegularExpression regexp(R"(^(?:([\w<>, ]+) )?(\w+)::(\w+)\((.*)\)$)");
    auto match = regexp.match(line);
    result.method.returnType = match.captured(1);
    result.qmlType = match.captured(2);
    result.method.name = match.captured(3);

    const QStringList parameters = match.captured(4).split(',');
    for (auto text : parameters) {
        Data::QmlMethod::Parameter param;
        text = text.simplified();
        if (text.isEmpty())
            continue;

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
            definition = parseMethodDefinition(line.mid(11));
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
    else if (line.startsWith("\\todo"))
        block.isExperimental = true;
    else if (line.startsWith("\\ingroup")) {
        auto strings = line.mid(9).split('/');
        if (strings.size() > 0)
            block.group = strings.first();
        if (strings.size() > 1) {
            if (strings.at(1) == "@first")
                block.positionInGroup = Data::FirstInGroup;
            else if (strings.at(1) == "@last")
                block.positionInGroup = Data::LastInGroup;
        }
    } else if (line.startsWith("\\"))
        return; // Tag not handled
    else if (line.isEmpty() && block.description.isEmpty())
        return;
    else
        block.description += line + "\n";
}
