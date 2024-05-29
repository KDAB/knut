/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "rccore/rcfile.h"

#include <QMainWindow>

namespace RcUi {
class RcFileView;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void load();

private:
    RcUi::RcFileView *m_view = nullptr;
    RcCore::RcFile m_rcFile;
    QString m_fileName;
};
