/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "codedocument.h"

namespace Core {

class RustDocument : public CodeDocument
{
    Q_OBJECT

public:
    explicit RustDocument(QObject *parent = nullptr);
    ~RustDocument() override;
};

}
