#include "core/cppdocument.h"
#include "core/knutcore.h"
#include "core/project.h"

#include "common/test_utils.h"

#include <QTest>
#include <QThread>

class TestCppDocument : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

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

        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/cppdocument");
        auto document = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(headerOrSource));
        QCOMPARE(sourceOrHeader, document->correspondingHeaderSource());
    }

    void insertForwardDeclInHeader()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/cppdocument");

        Test::FileTester file(Test::testDataPath() + "/cppdocument/test/forward_declaration_original.h");
        Core::CppDocument headerFile;
        headerFile.load(file.fileName());
        QCOMPARE(headerFile.insertForwardDeclaration("class Foo"), true);
        QCOMPARE(headerFile.insertForwardDeclaration("class Foo::Bar::FooBar"), true);
        QCOMPARE(headerFile.insertForwardDeclaration("class Foo::Bar::FooBar"), false);
        QCOMPARE(headerFile.insertForwardDeclaration(" "), false);
        QCOMPARE(headerFile.insertForwardDeclaration("Foo::Bar::FooBar"), false);
        QCOMPARE(headerFile.insertForwardDeclaration("struct Neo"), true);
        QCOMPARE(headerFile.insertForwardDeclaration("struct SFoo::SBar::Uno"), true);
        headerFile.save();
        QVERIFY(file.compare());
        headerFile.close();
    }

    void extractDataExchange()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/mfc/tutorial");

        auto document = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open("TutorialDlg.cpp"));
        auto ddxMap = document->mfcExtractDDX("CTutorialDlg");

        QCOMPARE(ddxMap.size(), 8);
        QCOMPARE(ddxMap.value("IDC_ECHO_AREA"), "m_EchoText");
        QCOMPARE(ddxMap.value("IDC_MOUSEECHO"), "m_MouseEcho");
    }

    void getSymbols()
    {
        CHECK_CLANGD_VERSION;

        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/mfc/tutorial");

        auto cppDocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open("TutorialDlg.cpp"));
        const auto cppSymbols = cppDocument->symbols();

        QCOMPARE(cppSymbols.size(), 15);
        const auto constructor = cppSymbols.first();
        QCOMPARE(constructor.kind, Core::Symbol::Constructor);
        QCOMPARE(constructor.name, "CTutorialDlg::CTutorialDlg");
        cppDocument->selectRange(constructor.selectionRange);
        const QString constructorName = cppDocument->selectedText();
        QCOMPARE(constructorName, "CTutorialDlg");
        cppDocument->selectRange(constructor.range);
        const QString constructorCode = cppDocument->selectedText();
        QVERIFY(constructorCode.startsWith("CTutorialDlg::CTutorialDlg(CWnd* pParent)"));
        QVERIFY(constructorCode.endsWith('}'));

        QCOMPARE(cppSymbols.at(1).kind, Core::Symbol::Method);
        QCOMPARE(cppSymbols.at(1).name, "CTutorialDlg::DoDataExchange");
        QCOMPARE(cppSymbols.at(1).description, "void (CDataExchange *)");

        auto hDocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open("TutorialDlg.h"));
        const auto hSymbols = hDocument->symbols();

        QCOMPARE(hSymbols.size(), 26);
        QCOMPARE(hSymbols.first().kind, Core::Symbol::Class);
        QCOMPARE(hSymbols.first().name, "CTutorialDlg");

        QCOMPARE(hSymbols.last().kind, Core::Symbol::Field);
        QCOMPARE(hSymbols.last().name, "CTutorialDlg::m_hIcon");
        hDocument->selectRange(hSymbols.last().range);
        QCOMPARE(hDocument->selectedText(), "HICON m_hIcon");
    }
};

QTEST_MAIN(TestCppDocument)
#include "tst_cppdocument.moc"
