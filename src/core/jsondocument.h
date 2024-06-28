/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "textdocument.h"
#include "utils/log.h"

namespace Core {

class JsonDocument : public TextDocument
{
    Q_OBJECT

public:
    explicit JsonDocument(QObject *parent = nullptr);
    ~JsonDocument() override;

    Q_INVOKABLE bool hasValue(const QString &path) const;
    Q_INVOKABLE QVariant value(const QString &path, const QVariant &defaultValue = {}) const;

    template <typename T>
    T value(std::string path) const
    {
        if (!path.starts_with('/'))
            path = '/' + path;
        try {
            return m_jsonData.at(nlohmann::json::json_pointer(path)).get<T>();
        } catch (...) {
            spdlog::error("JsonDocument::value {} - error reading", path);
        }
        return {};
    }

public slots:
    bool setValue(const QString &path, const QVariant &value);

protected:
    bool doSave(const QString &fileName) override;
    bool doLoad(const QString &fileName) override;

private:
    bool loadJsonData(const QString &fileName);
    nlohmann::json m_jsonData;
};

}
