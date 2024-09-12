/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "imagedocument.h"
#include "utils/log.h"

namespace Core {

ImageDocument::ImageDocument(QObject *parent)
    : Document(Type::Image, parent)
{
}

QImage ImageDocument::image() const
{
    return m_image;
}

bool ImageDocument::doSave(const QString &fileName)
{
    Q_UNUSED(fileName)
    spdlog::error("{}: not implemented yet", FUNCTION_NAME);
    return false;
}

bool ImageDocument::doLoad(const QString &fileName)
{
    return m_image.load(fileName);
}

} // namespace Core
