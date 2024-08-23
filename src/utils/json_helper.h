/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "json.h"

namespace Utils {

enum class LoadJsonStatus { Success, InvalidJson, UnreadableFile, UnknownError };

enum class SetJsonValueStatus { Success, NullValue, ValueTypeNotHandled };

enum class JsonValueStatus { Success, ValueUnknown, ConversionUnknown };

struct LoadJsonDataStatus
{
    std::optional<nlohmann::json> jsonData;
    LoadJsonStatus loadJsonStatus;
};

struct GetJsonValueStatus
{
    QVariant value;
    // Default
    JsonValueStatus jsonValueStatus = JsonValueStatus::Success;
};

LoadJsonDataStatus loadJsonData(const QString &fileName);

bool hasJsonValue(const nlohmann::json &jsonData, QString path);
SetJsonValueStatus setJsonValue(nlohmann::json &jsonData, QString path, const QVariant &value);
GetJsonValueStatus jsonValue(const nlohmann::json &jsonData, QString path, const QVariant &defaultValue = {});

} // namespace Utils
