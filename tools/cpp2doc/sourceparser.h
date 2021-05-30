#pragma once

#include "data.h"

class QTextStream;

class SourceParser
{
public:
    SourceParser(Data &data);

    void parseDirectory(const QString &directory);

private:
    QString cleanupCommentLine(QString line);
    void parseFile(const QString &fileName);

    Data::TypeBlock parseType(QTextStream &stream, QString line);
    Data::PropertyBlock parseProperty(QTextStream &stream, QString line);
    Data::MethodBlock parseMethod(QTextStream &stream, QString line);
    void parseBlock(const QString &line, Data::Block &block);

private:
    Data &m_data;
};
