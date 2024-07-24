/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick 2.12
import Script 1.0
import Script.Test 1.0

TestCase {
    name: "Project"
    id : sane
    age : 12

    sub : Another {
        id : 123
        callBack1: (mouse,x) => label.moveTo(mouse.x, mouse.y)
        callBack2: mouse => label.moveTo(mouse.x, mouse.y)
    }

    Standalone {
        NestedA {

        }
        NestedB {
            
        }
        NestedC {
            
        }
        NestedD {
            
        }
        NestedE {
            
        }
    }

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
    
    function add(a,b = undefined) {
        return a+b
    }
}
