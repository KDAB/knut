#include "testutil.h"

#include <QUrl>
#include <QtQml>

#include <QtQml/private/qqmlengine_p.h>

namespace Core {

/*!
 * \qmltype TestUtil
 * \brief Provides utility methods useful for testing.
 * \inqmlmodule Script.Test
 * \since 4.0
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

} // namespace Core
