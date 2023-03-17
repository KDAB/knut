#include "scriptitem.h"

namespace Core {

/*!
 * \qmltype Script
 * \brief Script object for writing non visual scripts.
 * \inqmlmodule Script
 * \ingroup Items
 * \since 1.0
 *
 * The `Script` is the base class for all creatable items in QML. It is needed as a `QtObject`
 * can't have any children in QML. It can be used as the basis for non visual QML scripts:
 *
 * ```qml
 * import Script 1.0
 *
 * Script {
 * // ...
 * }
 * ```
 */

ScriptItem::ScriptItem(QObject *parent)
    : QObject(parent)
{
}

ScriptItem::~ScriptItem() { }

QQmlListProperty<QObject> ScriptItem::data()
{
    return QQmlListProperty<QObject>(this, this, &ScriptItem::appendData, &ScriptItem::countData, &ScriptItem::atData,
                                     &ScriptItem::clearData);
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
