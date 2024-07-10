/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "scriptitem.h"
#include "scriptrunner.h"
#include "utils/log.h"

namespace Core {

/*!
 * \qmltype Script
 * \brief Script object for writing non visual scripts.
 * \inqmlmodule Knut
 * \ingroup Items
 *
 * The `Script` is the base class for all creatable items in QML. It is needed as a `QtObject`
 * can't have any children in QML. It can be used as the basis for non visual QML scripts:
 *
 * ```qml
 * import Knut
 *
 * Script {
 * // ...
 * }
 * ```
 *
 * You can also integrate unit-tests as part of the script dialog. This is done by adding methods prefixed with `test_`.
 * Those methods will be executed when the script is run in test mode (using `--test` command line option). If a test is
 * failing, the script will show a critical log message, and the script will return a non-zero exit code.
 *
 * ```qml
 * import Knut
 *
 * Script {
 *     function test_foo() {
 *         compare(1, 2, "test should failed")
 *     }
 *     function test_bar() {
 *         verify(1 == 1, "test should pass")
 *     }
 * }
 * ```
 */

ScriptItem::ScriptItem(QObject *parent)
    : QObject(parent)
{
}

ScriptItem::~ScriptItem() = default;

QQmlListProperty<QObject> ScriptItem::data()
{
    return QQmlListProperty<QObject>(this, this, &ScriptItem::appendData, &ScriptItem::countData, &ScriptItem::atData,
                                     &ScriptItem::clearData);
}

/*!
 * \qmlmethod Script::compare(var actual, var expected, string message = {})
 * Compare `actual` with `expected` and log an error `message` if they are not equal.
 *
 * This method will increment the number of failed test internally, and when the script is finished, the test runner
 * will return the number of failed tests.
 *
 * This is usually used in tests method like that:
 *
 * ```qml
 * Script {
 *     function test_foo() {
 *         compare(1, 2, "1 should be equal to 2"); // This will log an error
 *     }
 * }
 * ```
 */
void ScriptItem::compare(const QJSValue &actual, const QJSValue &expected, QString message)
{
    ScriptRunner::compare(this, actual, expected, message);
}

/*!
 * \qmlmethod Script::verify(bool value, string message = {})
 * Compare `actual` with `expected` and log an error `message` if they are not equal.
 *
 * This method will increment the number of failed test internally, and when the script is finished, the test runner
 * will return the number of failed tests.
 *
 * This is usually used in tests method like that:
 *
 * ```qml
 * Script {
 *     function test_foo() {
 *         let answerToEverything = 42;
 *         verify(answerToEverything == 42, "What else?");
 *     }
 * }
 * ```
 */
void ScriptItem::verify(bool value, QString message)
{
    ScriptRunner::verify(this, value, message);
}

void ScriptItem::appendData(QQmlListProperty<QObject> *list, QObject *obj)
{
    if (auto that = qobject_cast<ScriptItem *>(list->object)) {
        obj->setParent(that);
        that->m_data.push_back(obj);
        emit that->dataChanged();
    }
}

QObject *ScriptItem::atData(QQmlListProperty<QObject> *list, qsizetype index)
{
    if (auto that = qobject_cast<ScriptItem *>(list->object))
        return that->m_data.at(index);
    return nullptr;
}

qsizetype ScriptItem::countData(QQmlListProperty<QObject> *list)
{
    if (auto that = qobject_cast<ScriptItem *>(list->object))
        return static_cast<int>(that->m_data.size());
    return 0;
}

void ScriptItem::clearData(QQmlListProperty<QObject> *list)
{
    if (auto that = qobject_cast<ScriptItem *>(list->object)) {
        that->m_data.clear();
        emit that->dataChanged();
    }
}

} // namespace Core
