/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QObject>
#include <QVariant>

namespace Core {

class DynamicObject : public QObject
{
    // Replace the Q_OBJECT macro
public:
    QT_WARNING_PUSH
    Q_OBJECT_NO_OVERRIDE_WARNING
    const QMetaObject *metaObject() const override;
    void *qt_metacast(const char *) override;
    int qt_metacall(QMetaObject::Call, int, void **) override;
    QT_TR_FUNCTIONS
private:
    Q_OBJECT_NO_ATTRIBUTES_WARNING
    QT_WARNING_POP
    struct QPrivateSignal
    {
    };
    QT_ANNOTATE_CLASS(qt_qobject, "")

public:
    using QObject::QObject;
    ~DynamicObject() override;

    /** Adds a new property to the object, must be called before ready(). */
    void addProperty(const QByteArray &name, const QByteArray &type, QMetaType::Type typeId, const QVariant &value);

    /** Marks the object as ready and creates the metaObject. */
    void ready();

    /** Registers a callback to be called at every changes of a property. */
    using DataChangedFunc = std::function<void(const QString &, const QVariant &)>;
    void registerDataChangedCallback(DataChangedFunc func);

private:
    struct DynamicProperty
    {
        QByteArray name;
        QByteArray type;
        QMetaType::Type typeId;
        QVariant variant;
    };

    const QMetaObject *m_metaObject = &QObject::staticMetaObject;
    std::vector<DynamicProperty> m_properties;
    DataChangedFunc m_dataChangedCallback;
};

} // namespace Core
