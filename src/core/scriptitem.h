/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QJSValue>
#include <QObject>
#include <QQmlListProperty>
#include <vector>

namespace Core {

class ScriptItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<QObject> data READ data NOTIFY dataChanged FINAL)
    Q_PROPERTY(int failed MEMBER m_failed) // undocumented, internal use
    Q_CLASSINFO("DefaultProperty", "data")

public:
    explicit ScriptItem(QObject *parent = nullptr);
    ~ScriptItem() override;

    QQmlListProperty<QObject> data();

    Q_INVOKABLE void compare(const QJSValue &actual, const QJSValue &expected, QString message = {});
    Q_INVOKABLE void verify(bool value, QString message = {});

signals:
    void dataChanged();

private:
    static void appendData(QQmlListProperty<QObject> *list, QObject *obj);
    static QObject *atData(QQmlListProperty<QObject> *list, qsizetype index);
    static qsizetype countData(QQmlListProperty<QObject> *list);
    static void clearData(QQmlListProperty<QObject> *list);

private:
    std::vector<QObject *> m_data;
    int m_failed = 0;
};

} // namespace Core

Q_DECLARE_METATYPE(Core::ScriptItem *)
