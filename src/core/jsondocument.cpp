/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "jsondocument.h"
#include "logger.h"
#include "rcdocument.h"
#include "settings.h"
#include "utils/json_helper.h"

#include <QFile>
#include <QFileInfo>

namespace Core {

JsonDocument::JsonDocument(QObject *parent)
    : TextDocument(Type::Json, parent)
{
}

JsonDocument::~JsonDocument() = default;

bool JsonDocument::loadJsonData(const QString &fileName)
{
    auto loadJsonDataStatus = Utils::loadJsonData(fileName);
    switch (loadJsonDataStatus.loadJsonStatus) {
    case Utils::LoadJsonStatus::Success:
        break;
    case Utils::LoadJsonStatus::InvalidJson:
        spdlog::error("JsonDocument::loadJsonData {} - Invalid Json", fileName);
        break;
    case Utils::LoadJsonStatus::UnreadableFile:
        spdlog::error("JsonDocument::loadJsonData {} - File is not readable", fileName);
        break;
    case Utils::LoadJsonStatus::UnknownError:
        spdlog::error("JsonDocument::loadJsonData {} - Unknown error", fileName);
        break;
    }

    if (loadJsonDataStatus.jsonData) {
        m_jsonData = loadJsonDataStatus.jsonData.value();
        return true;
    }
    return false;
}

/*!
 * \qmlmethod bool JsonDocument::hasValue(string path)
 * Returns true if the json document has a name `path`.
 */
bool JsonDocument::hasValue(const QString &path) const
{
    LOG(path);
    return Utils::hasJsonValue(m_jsonData, path);
}

/*!
 * \qmlmethod variant JsonDocument::value(string path, variant defaultValue = null)
 * Returns the value of the name `path`, or `defaultValue` if the name does not exist.
 */
QVariant JsonDocument::value(const QString &path, const QVariant &defaultValue) const
{
    if (defaultValue.isValid())
        LOG(path, defaultValue);
    else
        LOG(path);

    // Special cases
    if (path == Settings::RcAssetColors || path == Settings::RcAssetFlags || path == Settings::RcDialogFlags) {
        return static_cast<int>(value<RcDocument::ConversionFlags>(path.toStdString()));
    }

    // Value of the name 'path' and getter status.
    const Utils::GetJsonValueStatus valueStatus = Utils::jsonValue(m_jsonData, path, defaultValue);

    switch (valueStatus.jsonValueStatus) {
    case Utils::JsonValueStatus::Success:
        return valueStatus.value;
    case Utils::JsonValueStatus::ValueUnknown:
        spdlog::info("JsonDocument::value {} - accessing non-existing value", path);
        break;
    case Utils::JsonValueStatus::ConversionUnknown:
        spdlog::error("JsonDocument::value {} - cannot convert", path);
        break;
    }
    return valueStatus.value;
}

/*!
 * \qmlmethod variant JsonDocument::setValue(string path, variant value)
 * Adds a new value `value` to the json document at the given name `path`. Returns `true` if the operation succeeded.
 */
bool JsonDocument::setValue(const QString &path, const QVariant &value)
{
    LOG(path, value);

    Utils::SetJsonValueStatus status = Utils::setJsonValue(m_jsonData, path, value);
    switch (status) {
    case Utils::SetJsonValueStatus::Success:
        setText(QString::fromStdString(m_jsonData.dump(4, ' ', false)));
        return true;
    case Utils::SetJsonValueStatus::NullValue:
        spdlog::error("JsonDocument::setValue {} in {} - value is null", value.toString(), path);
        break;
    case Utils::SetJsonValueStatus::ValueTypeNotHandled:
        spdlog::error("JsonDocument::setValue {} in {} - value type not handled", value.toString(), path);
        break;
    }
    return false;
}

bool JsonDocument::doLoad(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());

    if (TextDocument::doLoad(fileName)) {
        return loadJsonData(fileName);
    }
    return false;
}

bool JsonDocument::doSave(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());

    if (TextDocument::doSave(fileName)) {
        loadJsonData(fileName);
        return true;
    }
    return false;
}

} // namespace Core
