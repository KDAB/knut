#include "testutil.h"

#include <QFile>
#include <QUrl>
#include <QtQml>

#include <QtQml/private/qqmlengine_p.h>

namespace Core {

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

TestUtil::~TestUtil() { }

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
    QFile file1(file);
    if (!file1.open(QIODevice::ReadOnly))
        return false;
    QFile file2(expected);
    if (!file2.open(QIODevice::ReadOnly))
        return false;

    auto data1 = file1.readAll();
    auto data2 = file2.readAll();
    if (eolLF) {
        data1.replace("\r\n", "\n");
        data2.replace("\r\n", "\n");
    }
    return data1 == data2;
}

} // namespace Core
