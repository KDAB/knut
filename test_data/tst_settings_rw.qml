/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick
import Knut

Script {
    // We want to test the writing of settings without any project, that's why it's in a different QML file
    function test_setSettings() {
        verify(Settings.setValue("/int_val", 10))
        compare(Settings.value("/int_val"), 10)

        verify(Settings.setValue("double_val", 3.5))
        compare(Settings.value("double_val"), 3.5)

        verify(Settings.setValue("boolean/val", true))
        compare(Settings.value("boolean/val"), true)

        verify(Settings.setValue("/stringlistval", ["a", "b", "c"]))
        compare(Settings.value("/stringlistval"), ["a", "b", "c"])
    }

}
