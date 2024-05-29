/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "types.h"

#include <QString>
#include <QUrl>

namespace Lsp::Utils {

inline DocumentUri toDocumentUri(const QString &localFile)
{
    return QUrl::fromLocalFile(localFile).toString().toStdString();
}

QString removeTypeAliasInformation(const QString &typeInfo);

}
