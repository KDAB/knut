#pragma once

#include <QObject>
#include <QQmlListProperty>

#include <vector>

namespace Core {

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
using TypeSize = int;
#else
using TypeSize = qsizetype;
#endif

class ScriptItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<QObject> data READ data NOTIFY dataChanged FINAL)
    Q_CLASSINFO("DefaultProperty", "data")

public:
    explicit ScriptItem(QObject *parent = nullptr);
    ~ScriptItem() override;

    QQmlListProperty<QObject> data();

signals:
    void dataChanged();

private:
    static void appendData(QQmlListProperty<QObject> *list, QObject *obj);
    static QObject *atData(QQmlListProperty<QObject> *list, TypeSize index);
    static TypeSize countData(QQmlListProperty<QObject> *list);
    static void clearData(QQmlListProperty<QObject> *list);

private:
    std::vector<QObject *> m_data;
};

} // namespace Core

Q_DECLARE_METATYPE(Core::ScriptItem *)
