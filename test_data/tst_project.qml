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
        Project.root = Dir.currentScriptPath + "/projects/mfc-tutorial"

        var files = Project.allFiles();
        compare(files.length, 10)
        compare(files[0], "MFC_UpdateGUI.rc")

        var rcFiles = Project.allFilesWithExtension("rc");
        compare(rcFiles.length, 1)
        compare(rcFiles[0], "MFC_UpdateGUI.rc")
    }

    function test_open() {
        Project.root = Dir.currentScriptPath + "/projects/mfc-tutorial"

        var txtdoc = Project.open("TutorialDlg.cpp")
        compare(txtdoc.type, Document.Cpp)

        var rcdoc = Project.open("MFC_UpdateGUI.rc")
        compare(rcdoc.type, Document.Rc)
    }

    function test_findInFiles() {
        if(Project.isFindInFilesAvailable()) {
        let simplePattern = "CTutorialApp::InitInstance()"
        let simpleResults = Project.findInFiles(simplePattern)

            compare(simpleResults.length, 2)

            simpleResults.sort((a, b) => a.file.localeCompare(b.file));

            compare(simpleResults[0].file, Project.root + "/TutorialApp.cpp")
            compare(simpleResults[0].line, 21)
            compare(simpleResults[0].column, 6)
            compare(simpleResults[1].file, Project.root + "/TutorialDlg.h")
            compare(simpleResults[1].line, 10)
            compare(simpleResults[1].column, 9)

            let multilinePattern = "m_VSliderBar\\.SetRange\\(0,\\s*100,\\s*TRUE\\);\\s*m_VSliderBar\\.SetPos\\(50\\);";
            let multilineResults = Project.findInFiles(multilinePattern)

            compare(multilineResults.length, 1)

            compare(multilineResults[0].file, Project.root + "/TutorialDlg.cpp")
            compare(multilineResults[0].line, 65)
            compare(multilineResults[0].column, 3)
        }
        else {
            Message.warning("Ripgrep (rg) isn't available on the system")
        }
    }
}
