/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "scriptdialogitem_p.h"
#include "utils/log.h"

#include <private/qmetaobjectbuilder_p.h>

namespace Core {

DynamicObject::~DynamicObject()
{
    if (m_metaObject != &QObject::staticMetaObject)
        free(const_cast<QMetaObject *>(m_metaObject));
}

void DynamicObject::addProperty(const QByteArray &name, const QByteArray &type, QMetaType::Type typeId,
                                const QVariant &value)
{
    Q_ASSERT_X(m_metaObject == &QObject::staticMetaObject, "addProperty", "Can't add property after calling ready()");
    m_properties.emplace_back(DynamicProperty {name, type, typeId, value});
}

void DynamicObject::ready()
{
    Q_ASSERT_X(m_metaObject == &QObject::staticMetaObject, "ready", "ready() should be called only once.");

    QMetaObjectBuilder builder;
    builder.setSuperClass(&QObject::staticMetaObject);

    for (const auto &dynamicProperty : m_properties) {
        auto property = builder.addProperty(dynamicProperty.name, dynamicProperty.type, dynamicProperty.typeId);
        property.setWritable(true);
        auto signal = builder.addSignal(dynamicProperty.name + "Changed()");
        property.setNotifySignal(signal);
    }

    m_metaObject = builder.toMetaObject();
}

void DynamicObject::registerDataChangedCallback(DataChangedFunc func)
{
    m_dataChangedCallback = std::move(func);
}

const QMetaObject *DynamicObject::metaObject() const
{
    return m_metaObject;
}

int DynamicObject::qt_metacall(QMetaObject::Call call, int id, void **argv)
{
    const int realId = id - m_metaObject->propertyOffset();
    if (realId < 0) {
        return QObject::qt_metacall(call, id, argv);
    }

    if (call == QMetaObject::ReadProperty) {
        const auto &property = m_properties.at(static_cast<size_t>(realId));
        QMetaType metaType(property.typeId);
        metaType.construct(argv[0], property.variant.data());
    } else if (call == QMetaObject::WriteProperty) {
        auto &property = m_properties.at(static_cast<size_t>(realId));
        property.variant = QVariant(QMetaType(property.typeId), (void *)argv[0]);
        *reinterpret_cast<int *>(argv[2]) = 1; // setProperty return value
        QMetaObject::activate(this, m_metaObject, realId, nullptr);
        if (m_dataChangedCallback)
            m_dataChangedCallback(property.name, property.variant);
    } else {
        spdlog::warn("{}: id {} not handled.", FUNCTION_NAME, id);
    }

    return -1;
}

void *DynamicObject::qt_metacast(const char *name)
{
    if (strcmp(name, m_metaObject->className()) == 0) {
        return this;
    } else {
        return QObject::qt_metacast(name);
    }
}

} // namespace Core
