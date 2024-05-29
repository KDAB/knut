/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "qfileinfovaluetype.h"

#include <QObject>

namespace Core {

class FileInfo : public QObject
{
    Q_OBJECT

public:
    explicit FileInfo(QObject *parent = nullptr);
    ~FileInfo() override;

public slots:
    static bool exists(const QString &file);

    static Core::QFileInfoValueType create(const QString &file);
};

} // namespace Core
