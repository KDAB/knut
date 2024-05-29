/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QObject>

namespace Core {

class Message : public QObject
{
    Q_OBJECT

public:
    explicit Message(QObject *parent = nullptr);
    ~Message() override;

public slots:
    void error(const QString &text);
    void log(const QString &text);
    void debug(const QString &text);
    void warning(const QString &text);
};

} // namespace Core
