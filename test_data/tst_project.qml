import QtQuick 2.12
import Script 1.0
import Script.Rc 1.0
import Script.Test 1.0

TestCase {
    name: "Project"

    function test_allFiles() {
        Project.root = Dir.currentScriptPath + "/mfc/dialog"

        var files = Project.allFiles();
        compare(files.length, 12)
        compare(files[0], "dialog.aps")

        var rcFiles = Project.allFilesWithExtension("rc");
        compare(rcFiles.length, 1)
        compare(rcFiles[0], "dialog.rc")
    }

    function test_open() {
        Project.root = Dir.currentScriptPath + "/mfc/dialog"

        var txtdoc = Project.open("dialog.cpp")
        compare(txtdoc.type, Document.Cpp)

        var rcdoc = Project.open("dialog.rc")
        compare(rcdoc.type, Document.Rc)
    }

}
