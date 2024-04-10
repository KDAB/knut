import QtQuick 2.12
import Script 1.0
import Script.Test 1.0

import "../scripts/"

TestCase {
    name: "Photon Convert File"

    function test_conversion() {
        let testDir = TestUtil.createTestProjectFrom(TestUtil.testDataPath + "/tst_knut/photon/source");
        Project.root = testDir;

        let files = [
            //"Demonstration.cpp",
            "SimplePhotonFunctions.cpp",
            "PtTextGetSetSelection.cpp",
            "PtSetResources2.cpp",
            "PtSetResources.cpp",
            "PtSetResource.cpp",
            "PtRealizeWidget.cpp",
            "PtGetResources.cpp",
            "PtCreateWidgetEdge.cpp",
            "PtCreateWidget3.cpp",
            "PtCreateWidget2.cpp",
            "PtCreateWidget.cpp",
            "PtAddCallback.cpp",
            "PgColors.cpp",
            "GetSetResources.cpp",
            "PtCreateWidget4.cpp"
        ];

        for (let file of files) {
            Project.open(file);
            dialog.run();
        }

        Project.saveAllDocuments();
        verify(TestUtil.compareDirectories(testDir,TestUtil.testDataPath + "/tst_knut/photon/expected"));
        TestUtil.removeTestProject(testDir);
    }

    property PhotonConvertFile dialog: PhotonConvertFile {
        interactive: false
    }
}
