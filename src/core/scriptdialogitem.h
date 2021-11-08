#pragma once

#include <QDialog>
#include <QQmlListProperty>
#include <QQmlPropertyMap>

#include <vector>

namespace Core {

class ScriptDialogItem : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QQmlPropertyMap *data READ data NOTIFY dataChanged)
    Q_PROPERTY(QQmlListProperty<QObject> childrenData READ childrenData NOTIFY childrenDataChanged FINAL)
    Q_CLASSINFO("DefaultProperty", "childrenData")

public:
    explicit ScriptDialogItem(QWidget *parent = nullptr);

    QQmlPropertyMap *data() const;
    QQmlListProperty<QObject> childrenData();

signals:
    void clicked(const QString &name);
    void dataChanged();
    void childrenDataChanged();

private:
    void setUiFile(const QString &fileName);
    void changeValue(const QString &key, const QVariant &value);

    static void appendChild(QQmlListProperty<QObject> *list, QObject *obj);
    static QObject *atChild(QQmlListProperty<QObject> *list, int index);
    static int countChildren(QQmlListProperty<QObject> *list);
    static void clearChildren(QQmlListProperty<QObject> *list);

private:
    QQmlPropertyMap *m_data;
    std::vector<QObject *> m_children;
};

} // namespace Core

Q_DECLARE_METATYPE(QQmlPropertyMap *);
