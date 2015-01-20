/**********************************************************************
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <QStringList>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

#include <cstdlib>
#include <cstdio>

#include "rc2ui.h"

using namespace std;

static void panic(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");

    exit(EXIT_FAILURE);
}

static void usage()
{
    panic("Usage: rc2ui resource <include file>");
}

static QHash<int, QString> extractBMPIds(const QString &filename)
{
    QHash<int, QString> ids;

    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
        panic("Cannot open header file '%s'", filename.toLatin1().constData());

    QTextStream stream(&file);

    while (!stream.atEnd()) {
        const QString line = stream.readLine();

        if (!line.startsWith(QStringLiteral("#define")))
            continue;

        if (!line.contains(QStringLiteral("IDB_")))
            continue;

        QStringList fields = line.split(QStringLiteral(" "), QString::SkipEmptyParts);

        if (fields.size() < 3)
            continue;

        if (fields.at(2).isEmpty())
            continue;

        bool ok;

        const int key = fields.at(2).toInt(&ok);

        if (!ok)
            continue;

        ids.insert(key, fields.at(1));
    }

    return ids;
}

static QStringList import(const QString &filename, const QString &headerFile)
{
    QHash<int, QString> bmpIds;

    if (!headerFile.isEmpty()) {
        if(!QFileInfo::exists(headerFile))
            panic("Cannot find header file '%s'", headerFile.toLatin1().constData());

        bmpIds = extractBMPIds(headerFile);
    }


    QFile rcFile(filename);

    if (!rcFile.open(QIODevice::ReadOnly))
        panic("Could not open rcFile '%s' ", filename.toLatin1().constData());

    QTextStream in(&rcFile);

    RC2UI c(&in, bmpIds);
    QStringList files;
    c.parse();
    return c.targetFiles;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        usage();

    const QString rcFile = argv[1];
    const QString headerFile = (argc > 2) ? argv[2] : "";

    import(rcFile, headerFile);

    return 0;
}


