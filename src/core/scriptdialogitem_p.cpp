#include "scriptdialogitem_p.h"
#include "utils/log.h"

#include <private/qmetaobjectbuilder_p.h>

namespace Core {

DynamicObject::~DynamicObject()
{
    free(m_metaObject);
}

void DynamicObject::addProperty(const QByteArray &name, const QByteArray &type, QMetaType::Type typeId,
                                const QVariant &value)
{
    Q_ASSERT_X(m_metaObject == nullptr, "addProperty", "Can't add property after calling ready()");
    m_properties.emplace_back(DynamicProperty {name, type, typeId, value});
}

void DynamicObject::ready()
{
    Q_ASSERT_X(m_metaObject == nullptr, "ready", "ready() should be called only once.");

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
        spdlog::warn("DynamicObject::qt_metacall: id {} not handled.", id);
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
