#pragma once

#include <QDialog>
#include <QQmlListProperty>
#include <QQmlPropertyMap>

#include <vector>

namespace Core {

class DynamicObject;

class ScriptDialogItem : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QObject *data READ data CONSTANT)
    Q_PROPERTY(QQmlListProperty<QObject> childrenData READ childrenData NOTIFY childrenDataChanged FINAL)
    Q_CLASSINFO("DefaultProperty", "childrenData")

public:
    explicit ScriptDialogItem(QWidget *parent = nullptr);

    QObject *data() const;
    QQmlListProperty<QObject> childrenData();

signals:
    void clicked(const QString &name);
    void childrenDataChanged();

private:
    void setUiFile(const QString &fileName);
    void createProperties(QWidget *dialogWidget);
    void changeValue(const QString &key, const QVariant &value);

    static void appendChild(QQmlListProperty<QObject> *list, QObject *obj);
    static QObject *atChild(QQmlListProperty<QObject> *list, qsizetype index);
    static qsizetype countChildren(QQmlListProperty<QObject> *list);
    static void clearChildren(QQmlListProperty<QObject> *list);

private:
    DynamicObject *m_data;
    std::vector<QObject *> m_children;
};

} // namespace Core
