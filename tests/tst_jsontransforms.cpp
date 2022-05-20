#include "core/knutcore.h"
#include "core/lspdocument.h"
#include "core/project.h"

#include <QTest>
#include <qtestcase.h>

#include "common/test_utils.h"

class TestJsonTransforms : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void regexPatterns()
    {
        CHECK_CLANGD_VERSION;

        Test::FileTester tester(Test::testDataPath() + "/tst_jsontransforms/regexPattern/dot-to-arrow.cpp");
        {
            Core::KnutCore core;
            auto project = Core::Project::instance();
            project->setRoot(Test::testDataPath() + "/tst_jsontransforms/regexPattern");

            auto cppDocument = qobject_cast<Core::LspDocument *>(project->open(tester.fileName()));

            cppDocument->transformSymbol("object",
                                         Test::testDataPath() + "/tst_jsontransforms/regexPattern/dot-to-arrow.json");
            cppDocument->save();

            QVERIFY(tester.compare());
        }
    }
};

QTEST_MAIN(TestJsonTransforms)
#include "tst_jsontransforms.moc"
