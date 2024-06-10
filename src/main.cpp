/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "core/version.h"
#include "gui/knutmain.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("KDAB");
    QApplication::setApplicationName("knut");
    QApplication::setApplicationVersion(core::knut_version());
    QApplication::setWindowIcon(QIcon(":/gui/icons/knut-64.png"));

    Q_INIT_RESOURCE(core);
    Q_INIT_RESOURCE(rccore);
    Q_INIT_RESOURCE(gui);

    Gui::KnutMain knut;
    knut.process(app.arguments());

    return app.exec();
}
