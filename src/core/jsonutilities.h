/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "utils/json.h"

#include <QObject>

namespace Core {

class JsonUtilities : public QObject
{
    Q_OBJECT

public:
    explicit JsonUtilities(QObject *parent = nullptr);
    ~JsonUtilities() override;

    static std::optional<nlohmann::json> loadJsonData(const QString &fileName, bool log = true);

    bool hasValue(const nlohmann::json &jsonData, QString path) const;
    bool setValue(nlohmann::json &jsonData, QString path, const QVariant &value);
    QVariant value(const nlohmann::json &jsonData, QString path, const QVariant &defaultValue = {}) const;
};

} // namespace Core
