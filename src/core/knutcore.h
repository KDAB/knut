/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "settings.h"

#include <QCommandLineParser>
#include <QObject>

namespace Core {

class KnutCore : public QObject
{
    Q_OBJECT

public:
    explicit KnutCore(QObject *parent = nullptr);

    bool process(const QStringList &arguments);

protected:
    // Used to disambiguate the internal constructor
    struct InternalTag
    {
    };
    explicit KnutCore(InternalTag, QObject *parent = nullptr);

    virtual void initParser(QCommandLineParser &parser) const;
    virtual void doParse(const QCommandLineParser &parser) const;

private:
    void initialize(Settings::Mode mode);
    void initializeMultiSinkLogger();

    bool m_initialized = false;
};

} // namespace Core
