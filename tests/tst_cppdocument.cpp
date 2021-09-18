#include "core/cppdocument.h"
#include "core/knutcore.h"
#include "core/project.h"

#include "common/test_utils.h"

#include <QTest>

class TestCppDocument : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        Q_INIT_RESOURCE(core);
        core = new Core::KnutCore(this);
    }

    void correspondingHeaderSource_data()
    {
        QTest::addColumn<QString>("headerOrSource");
        QTest::addColumn<QString>("sourceOrHeader");

        QTest::newRow("cpp") << Test::testDataPath() + "/cppdocument/test/hello.cpp"
                             << Test::testDataPath() + "/cppdocument/test/hello.h";
        QTest::newRow("h") << Test::testDataPath() + "/cppdocument/test/hello.h"
                           << Test::testDataPath() + "/cppdocument/test/hello.cpp";
        QTest::newRow("cxx") << Test::testDataPath() + "/cppdocument/test/world.cxx"
                             << Test::testDataPath() + "/cppdocument/test/world.hpp";
        QTest::newRow("folder") << Test::testDataPath() + "/cppdocument/folder1/foo.cpp"
                                << Test::testDataPath() + "/cppdocument/folder2/foo.h";
        QTest::newRow("subfolder") << Test::testDataPath() + "/cppdocument/test/subfolder1/foo.cpp"
                                   << Test::testDataPath() + "/cppdocument/test/subfolder2/foo.h";
        QTest::newRow("notexist") << Test::testDataPath() + "/cppdocument/test/bar.cpp"
                                  << "";
    }

    void correspondingHeaderSource()
    {
        QFETCH(QString, headerOrSource);
        QFETCH(QString, sourceOrHeader);

        Core::Project::instance()->setRoot(Test::testDataPath() + "/cppdocument");
        auto document = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(headerOrSource));
        QCOMPARE(sourceOrHeader, document->correspondingHeaderSource());
    }

private:
    Core::KnutCore *core;
};

QTEST_MAIN(TestCppDocument)
#include "tst_cppdocument.moc"
