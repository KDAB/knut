import QtQuick 2.12
import Script 1.0
import Script.Test 1.0

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
