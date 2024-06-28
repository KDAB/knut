/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "jsondocument.h"
#include "jsonutilities.h"
// #include "document.h"
#include "rcdocument.h"
#include "settings.h"

namespace Core {

JsonDocument::JsonDocument(QObject *parent)
    : TextDocument(Type::Json, parent)
{
    connect(this, &JsonDocument::fileNameChanged, this, &JsonDocument::loadJsonData);
}

JsonDocument::~JsonDocument() = default;

void JsonDocument::loadJsonData()
{
    auto jsonData = JsonUtilities::loadJsonData(fileName());
    if (jsonData)
        m_jsonData = jsonData.value();
}

/*!
 * \qmlmethod bool JsonDocument::hasValue(string path)
 * Returns true if the json document has a name `path`.
 */
bool JsonDocument::hasValue(const QString &path) const
{
    return JsonUtilities().hasValue(m_jsonData, path);
}

/*!
 * \qmlmethod variant JsonDocument::value(string path, variant defaultValue = null)
 * Returns the value of the name `path`, or `defaultValue` if the name does not exist.
 */
QVariant JsonDocument::value(const QString &path, const QVariant &defaultValue) const
{
    // Special cases
    if (path == Settings::RcAssetColors || path == Settings::RcAssetFlags || path == Settings::RcDialogFlags) {
        return static_cast<int>(value<RcDocument::ConversionFlags>(path.toStdString()));
    }
    return JsonUtilities().value(m_jsonData, path, defaultValue);
}

/*!
 * \qmlmethod variant JsonDocument::setValue(string path, variant value)
 * Adds a new value `value` to the json document at the given name `path`. Returns `true` if the operation succeeded.
 */
bool JsonDocument::setValue(const QString &path, const QVariant &value)
{
    return JsonUtilities().setValue(m_jsonData, path, value);
}

} // namespace Core
