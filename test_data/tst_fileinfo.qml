/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick
import Knut
import Knut.Test

TestCase {
    name: "FileInfo"

    function endsWith(str,pattern) {
        return str.substr(str.length-pattern.length,pattern.length) === pattern
    }

    function startsWith(str,pattern) {
        return str.substr(0,pattern.length) === pattern
    }

    function test_fileInfo() {
        var absolute = FileInfo.create(Dir.currentScriptPath + "/tst_fileinfo/test.cpp")
        var relative = FileInfo.create("relative")

        compare(absolute.exists, true)
        compare(relative.exists, false)

        compare(relative.filePath, "relative")
        verify(endsWith(absolute.filePath, "test.cpp"))

        if (Qt.platform.os !== "windows")
            verify(startsWith(relative.absoluteFilePath,"/"))
        compare(absolute.fileName, "test.cpp")

        compare(absolute.isRelative, false)
        compare(relative.isRelative, true)
        if (Qt.platform.os !== "windows")
            compare(absolute.size, 49)
    }
}
