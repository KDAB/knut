/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "json_helper.h"

#include <QFile>
#include <optional>

namespace Utils {

LoadJsonDataStatus loadJsonData(const QString &fileName)
{
    LoadJsonDataStatus loadDataStatus;

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        try {
            auto jsonData = nlohmann::json::parse(file.readAll().constData());
            return {loadDataStatus.jsonData = jsonData, loadDataStatus.loadJsonStatus = LoadJsonStatus::Success};
        } catch (...) {
            return {loadDataStatus.jsonData = {}, loadDataStatus.loadJsonStatus = LoadJsonStatus::InvalidJson};
        }
    } else {
        return {loadDataStatus.jsonData = {}, loadDataStatus.loadJsonStatus = LoadJsonStatus::UnreadableFile};
    }
    return {loadDataStatus.jsonData = {}, loadDataStatus.loadJsonStatus = LoadJsonStatus::UnknownError};
}

bool hasJsonValue(const nlohmann::json &jsonData, QString path)
{
    if (!path.startsWith('/'))
        path.prepend('/');
    return jsonData.contains(nlohmann::json::json_pointer(path.toStdString()));
}

SetJsonValueStatus setJsonValue(nlohmann::json &jsonData, QString path, const QVariant &value)
{
    if (value.isNull()) {
        return SetJsonValueStatus::NullValue;
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
    case QMetaType::QVariantList:
        jsonData[jsonPath] = value.toStringList();
        break;
    default:
        return SetJsonValueStatus::ValueTypeNotHandled;
    }
    return SetJsonValueStatus::Success;
}

GetJsonValueStatus jsonValue(const nlohmann::json &jsonData, QString path, const QVariant &defaultValue)
{
    GetJsonValueStatus getJsonValue;
    getJsonValue.value = defaultValue;

    try {
        if (!path.startsWith('/'))
            path.prepend('/');

        auto val = jsonData.at(nlohmann::json::json_pointer(path.toStdString()));
        if (val.is_number_unsigned())
            return {getJsonValue.value = val.get<unsigned int>()};
        if (val.is_number_integer())
            return {getJsonValue.value = val.get<int>()};
        else if (val.is_number_float())
            return {getJsonValue.value = val.get<float>()};
        else if (val.is_boolean())
            return {getJsonValue.value = val.get<bool>()};
        else if (val.is_string())
            return {getJsonValue.value = val.get<QString>()};
        else if (val.is_array()) {
            // Only support QStringList for now
            if (!val.empty()) {
                if (val[0].is_string())
                    return {getJsonValue.value = val.get<QStringList>()};
            } else {
                return {getJsonValue.value = QStringList()};
            }
        } else {
            getJsonValue.jsonValueStatus = JsonValueStatus::ConversionUnknown;
        }
    } catch (...) {
        getJsonValue.jsonValueStatus = JsonValueStatus::ValueUnknown;
    }
    return getJsonValue;
}

} // namespace Utils
