#include "core/cppdocument.h"
#include "core/knutcore.h"
#include "core/project.h"
#include "core/scriptmanager.h"

#include "common/test_utils.h"

#include <QTemporaryFile>
#include <QTest>

class TestPhoton : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void conversion_data()
    {
        QTest::addColumn<QString>("source");

        QTest::addRow("SimplePhotonFunctions") << "SimplePhotonFunctions.cpp";
        QTest::addRow("PtTextGetSetSelection") << "PtTextGetSetSelection.cpp";
        QTest::addRow("PtSetResources2") << "PtSetResources2.cpp";
        QTest::addRow("PtSetResources") << "PtSetResources.cpp";
        QTest::addRow("PtSetResource") << "PtSetResource.cpp";
        QTest::addRow("PtRealizeWidget") << "PtRealizeWidget.cpp";
        QTest::addRow("PtGetResources") << "PtGetResources.cpp";
        QTest::addRow("PtCreateWidgetEdge") << "PtCreateWidgetEdge.cpp";
        QTest::addRow("PtCreateWidget3") << "PtCreateWidget3.cpp";
        QTest::addRow("PtCreateWidget2") << "PtCreateWidget2.cpp";
        QTest::addRow("PtCreateWidget") << "PtCreateWidget.cpp";
        QTest::addRow("PtAddCallback") << "PtAddCallback.cpp";
        QTest::addRow("PgColors") << "PgColors.cpp";
        QTest::addRow("GetSetResources") << "GetSetResources.cpp";
        QTest::addRow("PtCreateWidget4") << "PtCreateWidget4.cpp";
        // QTest::addRow("Demonstration") << "Demonstration";
    }

    void conversion()
    {
        QFETCH(QString, source);

        Test::LogSilencer ls;

        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/tst_photon");

        testSource(source);
    }

    void testSource(const QString &source)
    {
        Test::FileTester file(Test::testDataPath() + "/tst_photon/" + source);
        qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(file.fileName()));

        const auto scriptPath = QCoreApplication::applicationDirPath() + "/scripts/photon-convert-file.qml";
        Core::ScriptManager::instance()->runScript(scriptPath, false, true);
        QVERIFY(file.compare());
        Core::Project::instance()->closeAll();
    }
};

QTEST_MAIN(TestPhoton)
#include "tst_photon.moc"
