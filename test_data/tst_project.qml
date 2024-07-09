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
    name: "Project"

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

}
