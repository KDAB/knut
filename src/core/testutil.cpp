/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "testutil.h"
#include "utils/log.h"

#include <QDirIterator>
#include <QFile>
#include <QQmlEngine>
#include <QUrl>
#include <QtQml/private/qqmlengine_p.h>

namespace Core {

/*!
 * \qmltype TestUtil
 * \brief Provides utility methods useful for testing.
 * \inqmlmodule Knut.Test
 *
 * This class is mainly used by the [TestCase](testcase.md) object, to extract some information on the script.
 *
 * It should not be used in normal scripts.
 */

TestUtil::TestUtil(QObject *parent)
    : QObject(parent)
{
}

TestUtil::~TestUtil() = default;

QString TestUtil::callerFile(int frameIndex) const
{
    QQmlEngine *engine = qmlEngine(this);
    QV4::ExecutionEngine *v4 = QQmlEnginePrivate::getV4Engine(engine);

    const QList<QV4::StackFrame> stack = v4->stackTrace(frameIndex + 2);
    if (stack.size() > frameIndex + 1) {
        return QDir::toNativeSeparators(QUrl(stack.at(frameIndex + 1).source).toLocalFile());
    }
    return QString();
}

int TestUtil::callerLine(int frameIndex) const
{
    QQmlEngine *engine = qmlEngine(this);
    QV4::ExecutionEngine *v4 = QQmlEnginePrivate::getV4Engine(engine);

    QList<QV4::StackFrame> stack = v4->stackTrace(frameIndex + 2);
    if (stack.size() > frameIndex + 1)
        return stack.at(frameIndex + 1).line;
    return -1;
}

/*!
 * \qmlmethod TestUtil::compareFiles(string file, string expected, bool eolLF = true)
 * Compares the content of the two files, and return true if they are the same.
 * If `eolLF` true, compareFiles will change the EOL of files to LF for comparison.
 */
bool TestUtil::compareFiles(const QString &file, const QString &expected, bool eolLF)
{
    QFile file1(file);
    if (!file1.open(QIODevice::ReadOnly)) {
        spdlog::warn("Cannot open {} for comparison!", file);
        return false;
    }
    QFile file2(expected);
    if (!file2.open(QIODevice::ReadOnly)) {
        spdlog::warn("Cannot open {} for comparison!", expected);
        return false;
    }

    auto data1 = file1.readAll();
    auto data2 = file2.readAll();
    if (eolLF) {
        data1.replace("\r\n", "\n");
        data2.replace("\r\n", "\n");
    }
    auto result = data1 == data2;
    if (!result) {
        spdlog::warn("Comparison of {} and {} failed!", file, expected);
        spdlog::warn("Actual: {}", data1);
        spdlog::warn("Expected: {}", data2);
    }
    return result;
}

/*!
 * \qmlmethod TestUtil::createTestProjectFrom(string path)
 * Creates a new recursive copy of the directory at `path` with the name `path`_test_data.
 * Returns the newly created directory path.
 */
QString TestUtil::createTestProjectFrom(const QString &path)
{
    QDir fromDir(path);

    Q_ASSERT(fromDir.exists());

    QDirIterator it(fromDir, QDirIterator::Subdirectories);
    QDir toDir(fromDir.absolutePath() + "_test_data");
    toDir.removeRecursively();
    toDir.mkpath(toDir.absolutePath());
    auto absSourcePathLength = fromDir.absolutePath().length();

    while (it.hasNext()) {
        it.next();
        const auto fileInfo = it.fileInfo();
        const QString subPathStructure = fileInfo.absoluteFilePath().mid(absSourcePathLength);
        const QString fullPath = toDir.absolutePath() + subPathStructure;

        if (fileInfo.isDir()) {
            toDir.mkpath(fullPath);
        } else if (fileInfo.isFile()) {
            QFile::copy(fileInfo.absoluteFilePath(), fullPath);
        }
    }

    return toDir.absolutePath();
}

/*!
 * \qmlmethod TestUtil::removeTestProject(string path)
 * Removes the `path` from the disk.
 */
void TestUtil::removeTestProject(const QString &path)
{
    QDir dir(path);
    dir.removeRecursively();
}

/*!
 * \qmlmethod TestUtil::compareDirectories(string current, string expected)
 * Compares the two directories recursively, and return true if they are the same.
 */
bool TestUtil::compareDirectories(const QString &current, const QString &expected)
{
    QDir currentDir(current);
    if (!currentDir.exists()) {
        spdlog::warn("Cannot open directory {} for comparison!", current);
        return false;
    }

    bool result = true;

    QDirIterator it(currentDir, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();

        const auto fileInfo = it.fileInfo();

        if (fileInfo.isDir())
            continue;

        const QString subPath = fileInfo.absoluteFilePath().mid(currentDir.absolutePath().length());

        result &= compareFiles(fileInfo.absoluteFilePath(), expected + subPath);
    }

    return result;
}

} // namespace Core
