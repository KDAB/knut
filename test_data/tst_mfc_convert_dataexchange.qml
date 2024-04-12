import QtQuick 2.12
import Script 1.0
import Script.Test 1.0

import "../scripts/lib/data_exchange2qt.js" as DataExchange

TestCase {
    name: "MFC Convert DataExchange"

    function test_conversion() {
        let testDir = TestUtil.createTestProjectFrom(Dir.currentScriptPath + "/tst_dataexchange/original");
        Project.root = testDir;
        let cppfile = Project.open("DataExchange.cpp");
        let headerfile = cppfile.openHeaderSource();

        DataExchange.convert({
            cpp: cppfile,
            header: headerfile,
        }, "TestClass");

        Project.saveAllDocuments();
        verify(TestUtil.compareDirectories(testDir,Dir.currentScriptPath + "/tst_dataexchange/expected"));
        TestUtil.removeTestProject(testDir);
    }
}
