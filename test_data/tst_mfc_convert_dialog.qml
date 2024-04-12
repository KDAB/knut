import QtQuick 2.12
import Script 1.0
import Script.Test 1.0

import "../scripts/"

TestCase {
    name: "MFC Convert Dialog"

    function test_conversion() {
        let testDir = TestUtil.createTestProjectFrom(Dir.currentScriptPath + "/projects/mfc-tutorial");
        Project.root = testDir;
        Project.open("TutorialDlg.cpp");

        dialog.init();
        dialog.data.className = "CTutorialDlg";
        dialog.convert();

        Project.saveAllDocuments();
        verify(TestUtil.compareDirectories(testDir,Dir.currentScriptPath + "/tst_knut/mfc-convert-dialog_expected"));
        TestUtil.removeTestProject(testDir);
    }

    property MFCConvertDialog dialog: MFCConvertDialog {
        interactive: false
        uiFilePath: "../scripts/mfc-deprecated-convert-dialog.ui"
    }
}
