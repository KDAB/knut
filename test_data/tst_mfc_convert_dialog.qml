import QtQuick 2.12
import Script 1.0
import Script.Test 1.0

import "../scripts/"

TestCase {
    name: "MFC Convert Dialog"

    function test_conversion() {
        let testDir = TestUtil.createTestProjectFrom(TestUtil.testDataPath + "/projects/mfc-tutorial");
        Project.root = testDir;
        Project.open("TutorialDlg.cpp");

        dialog.init();
        dialog.data.className = "CTutorialDlg";
        dialog.convert();

        Project.saveAllDocuments();
        verify(TestUtil.compareDirectories(testDir,TestUtil.testDataPath + "/tst_knut/mfc-convert-dialog_expected"));
        TestUtil.removeTestProject(testDir);
    }

    property MFCConvertDialog dialog: MFCConvertDialog {
        uiFilePath: "../scripts/mfc-deprecated-convert-dialog.ui"
    }
}
