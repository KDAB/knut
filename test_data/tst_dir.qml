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
    name: "Dir"

    function test_basic() {
        if (Qt.platform.os === "windows") {
            var windows = Dir.create("C:\\Users\\Public")
            compare(windows.path,"C:/Users/Public")
            compare(windows.absolutePath,"C:/Users/Public")
            compare(windows.dirName,"Public")
            compare(windows.isReadable, true)
            compare(windows.exists, true)
            compare(windows.isRelative, false)

        } else {
            var usrbin = Dir.create("/usr/bin")
            compare(usrbin.path,"/usr/bin")
            compare(usrbin.absolutePath,"/usr/bin")
            compare(usrbin.dirName,"bin")
            compare(usrbin.isReadable, true)
            compare(usrbin.exists, true)
            compare(usrbin.isRelative, false)

            var root = Dir.create("/")
            compare(usrbin.isRoot, false)
            compare(root.isRoot, true)
        }

        var bogus = Dir.create("/bogus")
        compare(bogus.exists, false)

        var relative = Dir.create("foo")
        compare(relative.isRelative, true)
    }

    function test_dirOp() {
        var scriptDir = Dir.currentScript()

        compare(scriptDir.fileExists("NEWDIR"),false)
        verify(scriptDir.mkdir("NEWDIR"))

        compare(scriptDir.fileExists("NEWDIR"),true)
        verify(scriptDir.rename("NEWDIR", "NEWDIR2"))

        compare(scriptDir.fileExists("NEWDIR"),false)
        compare(scriptDir.fileExists("NEWDIR2"),true)
        verify(scriptDir.rmdir("NEWDIR2"))

        compare(scriptDir.fileExists("NEWDIR2"),false)
    }

    function test_filter() {
        var testcase = Dir.create(Dir.currentScriptPath + "/tst_dir")

        var files = testcase.entryList()
        compare(files.length, 7)

        files = testcase.entryList(Dir.Files, Dir.Name )
        compare(files,["a","b","c"])

        files = testcase.entryList(Dir.Dirs, Dir.Name )
        compare(files,[".","..","x","y"])

        files = testcase.entryList(Dir.Dirs | Dir.NoDotAndDotDot, Dir.Name )
        compare(files,["x","y"])

        files = testcase.entryList(Dir.Files | Dir.NoDotAndDotDot, Dir.Size)
        compare(files, ["c","b","a"])

        files = testcase.entryList(Dir.Files | Dir.NoDotAndDotDot, Dir.Size | Dir.Reversed)
        compare(files, ["a","b","c"])

        testcase = Dir.create(Dir.currentScriptPath + "/tst_dir/x" )
        files = testcase.entryList("*.txt", Dir.Files, Dir.Name )
        compare(files,["abc.txt","def.txt"])

        files = testcase.entryList(["*.txt", "*.data"], Dir.Files, Dir.Name )
        compare(files,["abc.txt","def.txt","hij.data","klm.data"])
    }

    function test_cd() {
        var dir = Dir.currentScript()
        var filesHere = dir.entryList()

        verify(dir.cd("tst_dir"))
        var files = dir.entryList(Dir.Files, Dir.Name )
        compare(files,["a","b","c"])

        verify(dir.cdUp())
        compare(filesHere, dir.entryList())
    }

    function test_staticFunctions() {
        var str
        if (Qt.platform.os === "windows") {
            str = Dir.toNativeSeparators("C:/Windows")
            compare(str, "C:\\Windows")
        } else {
            str = Dir.toNativeSeparators("/usr/bin")
            compare(str, "/usr/bin")
        }

        Dir.currentPath = Dir.currentScriptPath
        compare(Dir.currentPath, Dir.currentScriptPath)
        verify(Dir.current().fileExists("tst_dir.qml"))

        compare(Dir.cleanPath("/usr/./../tmp/./bah"),"/tmp/bah")
    }
}
