/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "textview.h"

namespace Core {
class QtTsDocument;
}

namespace Gui {
class QtTsView : public TextView
{
    Q_OBJECT
public:
    explicit QtTsView(QWidget *parent = nullptr);

    void setTsDocument(Core::QtTsDocument *document);

private:
    Core::QtTsDocument *m_document = nullptr;
};
}
