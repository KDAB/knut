#pragma once

#include <QObject>
#include <QQmlListProperty>

#include <vector>

namespace Core {

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
    static QObject *atData(QQmlListProperty<QObject> *list, qsizetype index);
    static qsizetype countData(QQmlListProperty<QObject> *list);
    static void clearData(QQmlListProperty<QObject> *list);

private:
    std::vector<QObject *> m_data;
};

} // namespace Core

Q_DECLARE_METATYPE(Core::ScriptItem *)
