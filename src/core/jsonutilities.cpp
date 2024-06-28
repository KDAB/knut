/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "jsonutilities.h"
#include "logger.h"
#include "utils/log.h"

#include <QFile>
#include <optional>

namespace Core {

JsonUtilities::JsonUtilities(QObject *parent)
    : QObject(parent)
{
}

JsonUtilities::~JsonUtilities() = default;

std::optional<nlohmann::json> JsonUtilities::loadJsonData(const QString &fileName, bool log)
{
    if (fileName.isEmpty())
        return {};

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        try {
            auto jsonData = nlohmann::json::parse(file.readAll().constData());
            if (log)
                spdlog::debug("JsonUtilities::loadData {}", fileName);
            return jsonData;
        } catch (...) {
            if (log)
                spdlog::error("JsonUtilities::loadData {}", fileName);
        }
    } else {
        if (log)
            spdlog::debug("JsonUtilities::loadData {} - file can't be read", fileName);
        return "{}"_json;
    }
    return {};
}

bool JsonUtilities::hasValue(const nlohmann::json &jsonData, QString path) const
{
    LOG("JsonUtilities::hasValue", path);

    if (!path.startsWith('/'))
        path.prepend('/');
    return jsonData.contains(nlohmann::json::json_pointer(path.toStdString()));
}

bool JsonUtilities::setValue(nlohmann::json &jsonData, QString path, const QVariant &value)
{
    LOG("JsonUtilities::setValue", path, value);

    if (value.isNull()) {
        spdlog::error("JsonUtilities::setValue {} in {} - value is null", value.toString(), path);
        return false;
    }

    if (!path.startsWith('/'))
        path.prepend('/');

    auto jsonPath = nlohmann::json::json_pointer(path.toStdString());
    switch (static_cast<QMetaType::Type>(value.typeId())) {
    case QMetaType::Bool:
        jsonData[jsonPath] = value.toBool();
        break;
    case QMetaType::Int:
    case QMetaType::LongLong:
        jsonData[jsonPath] = value.toInt();
        break;
    case QMetaType::UInt:
    case QMetaType::ULongLong:
        jsonData[jsonPath] = value.toUInt();
        break;
    case QMetaType::Double:
        jsonData[jsonPath] = value.toDouble();
        break;
    case QMetaType::QString:
        jsonData[jsonPath] = value.toString();
        break;
    case QMetaType::QStringList:
        jsonData[jsonPath] = value.toStringList();
        break;
    default:
        spdlog::error("JsonUtilities::setValue {} in {} - value type not handled", value.toString(), path);
        return false;
    }
    return true;
}

QVariant JsonUtilities::value(const nlohmann::json &jsonData, QString path, const QVariant &defaultValue) const
{
    if (defaultValue.isValid())
        LOG("JsonUtilities::value", path, defaultValue);
    else
        LOG("JsonUtilities::value", path);
    try {
        if (!path.startsWith('/'))
            path.prepend('/');

        auto val = jsonData.at(nlohmann::json::json_pointer(path.toStdString()));
        if (val.is_number_unsigned())
            return val.get<unsigned int>();
        if (val.is_number_integer())
            return val.get<int>();
        else if (val.is_number_float())
            return val.get<float>();
        else if (val.is_boolean())
            return val.get<bool>();
        else if (val.is_string())
            return val.get<QString>();
        else if (val.is_array()) {
            // Only support QStringList for now
            if (!val.empty()) {
                if (val[0].is_string())
                    return val.get<QStringList>();
            } else {
                return QStringList();
            }
        }
        spdlog::error("JsonUtilities::value {} - can't convert", path);
    } catch (...) {
        spdlog::info("JsonUtilities::value {} - accessing non-existing value", path);
    }
    return defaultValue;
}

} // namespace Core
