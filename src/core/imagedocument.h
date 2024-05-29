/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "document.h"

#include <QImage>

namespace Core {

class ImageDocument : public Document
{
    Q_OBJECT

public:
    explicit ImageDocument(QObject *parent = nullptr);

    QImage image() const;

protected:
    bool doSave(const QString &fileName) override;
    bool doLoad(const QString &fileName) override;

private:
    QImage m_image;
};

} // namespace Core
