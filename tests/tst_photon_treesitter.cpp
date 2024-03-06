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

        QTest::addRow("PgColors") << "PgColors.cpp";
        QTest::addRow("PtSetResources") << "PtSetResources.cpp";
        QTest::addRow("PtSetResources2") << "PtSetResources2.cpp";
        QTest::addRow("PtSetResource") << "PtSetResource.cpp";
    }

    void conversion()
    {
        QFETCH(QString, source);

        Test::LogSilencer ls;

        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/tst_photon_ts");

        testSource(source);
    }

    void testSource(const QString &source)
    {
        Test::FileTester file(Test::testDataPath() + "/tst_photon_ts/" + source);
        qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(file.fileName()));

        const auto scriptPath = QCoreApplication::applicationDirPath() + "/scripts/photon-convert-file-ts.qml";

        QBENCHMARK_ONCE {
            Core::ScriptManager::instance()->runScript(scriptPath, false, true);
        }

        QVERIFY(file.compare());
        Core::Project::instance()->closeAll();
    }
};

QTEST_MAIN(TestPhoton)
#include "tst_photon_treesitter.moc"
