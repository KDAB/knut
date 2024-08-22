/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "data.h"

class QTextStream;

class SourceParser
{
public:
    SourceParser(Data &data);

    void parseDirectory(const QString &directory);
    void loadMappingFile(const QString &filePath);

private:
    QString cleanupCommentLine(QString line);
    void parseFile(const QString &fileName);

    Data::TypeBlock parseType(QTextStream &stream, QString line);
    void updateFileMap(const QString &fileName, const Data::TypeBlock &currentType);
    Data::PropertyBlock parseProperty(QTextStream &stream, QString line);
    Data::MethodBlock parseMethod(QTextStream &stream, QString line);
    Data::SignalBlock parseSignal(QTextStream &stream, QString line);
    void parseBlock(const QString &line, Data::Block &block);

private:
    Data &m_data;
};
