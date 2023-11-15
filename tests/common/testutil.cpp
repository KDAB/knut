#include "testutil.h"

#include <QDirIterator>
#include <QFile>
#include <QQmlEngine>
#include <QUrl>

#include <QtQml/private/qqmlengine_p.h>

#include <spdlog/spdlog.h>

#include "test_utils.h"

/*!
 * \qmltype TestUtil
 * \brief Provides utility methods useful for testing.
 * \inqmlmodule Script.Test
 * \since 1.0
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

    const QVector<QV4::StackFrame> stack = v4->stackTrace(frameIndex + 2);
    if (stack.size() > frameIndex + 1) {
        return QDir::toNativeSeparators(QUrl(stack.at(frameIndex + 1).source).toLocalFile());
    }
    return QString();
}

int TestUtil::callerLine(int frameIndex) const
{
    QQmlEngine *engine = qmlEngine(this);
    QV4::ExecutionEngine *v4 = QQmlEnginePrivate::getV4Engine(engine);

    QVector<QV4::StackFrame> stack = v4->stackTrace(frameIndex + 2);
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
    return Test::compareFiles(file, expected, eolLF);
}

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

void TestUtil::removeTestProject(const QString &path)
{
    QDir dir(path);
    dir.removeRecursively();
}

bool TestUtil::compareDirectories(const QString &current, const QString &expected)
{
    QDir currentDir(current);
    if (!currentDir.exists()) {
        spdlog::error("Cannot open directory {} for comparison!", current.toStdString());
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

QString TestUtil::testDataPath() const
{
    return Test::testDataPath();
}
