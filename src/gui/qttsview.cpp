/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qttsview.h"
#include "core/qttsdocument.h"

namespace Gui {
QtTsView::QtTsView(QWidget *parent)
    : TextView(parent)
{
}

void QtTsView::setTsDocument(Core::QtTsDocument *document)
{
    Q_ASSERT(document);
    setDocument(document);
}

}
