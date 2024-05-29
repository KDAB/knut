/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qmldocument.h"

namespace Core {

QmlDocument::QmlDocument(QObject *parent)
    : TextDocument(Type::Qml, parent)
{
}

}
