/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick
import Knut

Script {
    function test_allFiles() {
        Project.root = Dir.currentScriptPath + "/projects/mfc-dialog"

        var files = Project.allFiles();
        compare(files.length, 12)
        compare(files[0], "Tutorial.cpp")

        var rcFiles = Project.allFilesWithExtension("rc");
        compare(rcFiles.length, 1)
        compare(rcFiles[0], "Tutorial.rc")
    }

    function test_open() {
        Project.root = Dir.currentScriptPath + "/projects/mfc-dialog"

        var txtdoc = Project.open("TutorialDlg.cpp")
        compare(txtdoc.type, Document.Cpp)

        var rcdoc = Project.open("Tutorial.rc")
        compare(rcdoc.type, Document.Rc)
    }

    function test_findInFiles() {
        if(Project.isFindInFilesAvailable()) {
        let simplePattern = "CTutorialApp::InitInstance()"
        let simpleResults = Project.findInFiles(simplePattern)

            compare(simpleResults.length, 2)

            simpleResults.sort((a, b) => a.file.localeCompare(b.file));

            compare(simpleResults[0].file, Project.root + "/Tutorial.cpp")
            compare(simpleResults[0].line, 38)
            compare(simpleResults[0].column, 6)
            compare(simpleResults[1].file, Project.root + "/TutorialDlg.h")
            compare(simpleResults[1].line, 9)
            compare(simpleResults[1].column, 9)

            let multilinePattern = "SetIcon\\(m_hIcon,\\s*TRUE\\);.*\\s*SetIcon\\(m_hIcon,\\s*FALSE\\);";
            let multilineResults = Project.findInFiles(multilinePattern)

            compare(multilineResults.length, 1)

            compare(multilineResults[0].file, Project.root + "/TutorialDlg.cpp")
            compare(multilineResults[0].line, 58)
            compare(multilineResults[0].column, 2)
        }
        else {
            Message.warning("Ripgrep (rg) isn't available on the system")
        }
    }
}
