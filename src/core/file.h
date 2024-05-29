/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QObject>

namespace Core {

class File : public QObject
{
    Q_OBJECT

public:
    explicit File(QObject *parent = nullptr);
    ~File() override;

public slots:
    static bool copy(const QString &fileName, const QString &newName);
    static bool exists(const QString &fileName);
    static bool remove(const QString &fileName);
    static bool rename(const QString &oldName, const QString &newName);

    static bool touch(const QString &fileName);

    static QString readAll(const QString &fileName);
};

} // namespace Core
