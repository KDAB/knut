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

    void verifySymbol(Core::CppDocument *document, const Core::Symbol &symbol, const QString &name,
                      Core::Symbol::Kind kind, const QString &selectionText)
    {
        QVERIFY(!symbol.isNull());
        QCOMPARE(symbol.name, name);
        QCOMPARE(symbol.kind, kind);
        document->selectRange(symbol.selectionRange);
        QCOMPARE(document->selectedText(), selectionText);
    }

    void symbols()
    {
        CHECK_CLANGD_VERSION;

        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/cpp-project");

        auto cppDocument = qobject_cast<Core::CppDocument *>(project->open("myobject.cpp"));
        const auto cppSymbols = cppDocument->symbols();
        QCOMPARE(cppSymbols.size(), 2);

        auto constructor = cppSymbols.first();
        verifySymbol(cppDocument, constructor, "MyObject::MyObject", Core::Symbol::Kind::Constructor, "MyObject");
        cppDocument->selectRange(constructor.range);
        QCOMPARE(cppDocument->selectedText(),
                 QString(
                     R"(MyObject::MyObject(const std::string& message)
  : m_message(message)
{})"));

        auto function = cppSymbols.last();
        verifySymbol(cppDocument, function, "MyObject::sayMessage", Core::Symbol::Kind::Method, "sayMessage");
        cppDocument->selectRange(function.range);
        QCOMPARE(cppDocument->selectedText(),
                 QString(
                     R"EOF(void MyObject::sayMessage() {
  std::cout << m_message << std::endl;
})EOF"));

        auto headerDocument = cppDocument->openHeaderSource();
        QVERIFY(headerDocument);
        QVERIFY(headerDocument->fileName().endsWith("myobject.h"));

        const auto headerSymbols = headerDocument->symbols();
        QCOMPARE(headerSymbols.size(), 4);

        verifySymbol(headerDocument, headerSymbols.at(0), "MyObject", Core::Symbol::Kind::Class, "MyObject");
        verifySymbol(headerDocument, headerSymbols.at(1), "MyObject::MyObject", Core::Symbol::Kind::Constructor,
                     "MyObject");
        verifySymbol(headerDocument, headerSymbols.at(2), "MyObject::sayMessage", Core::Symbol::Kind::Method,
                     "sayMessage");
        verifySymbol(headerDocument, headerSymbols.at(3), "MyObject::m_message", Core::Symbol::Kind::Field,
                     "m_message");
    }

    void findSymbol()
    {
        CHECK_CLANGD_VERSION;

        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/cpp-project");

        auto headerDocument = qobject_cast<Core::CppDocument *>(project->open("myobject.h"));

        auto symbol = headerDocument->findSymbol("MyObject", Core::TextDocument::FindWholeWords);
        verifySymbol(headerDocument, symbol, "MyObject", Core::Symbol::Kind::Class, "MyObject");

        symbol = headerDocument->findSymbol("m_message", Core::TextDocument::FindWholeWords);
        QVERIFY(symbol.isNull());

        symbol = headerDocument->findSymbol("m_message");
        verifySymbol(headerDocument, symbol, "MyObject::m_message", Core::Symbol::Kind::Field, "m_message");

        symbol = headerDocument->findSymbol("saymessage", Core::TextDocument::FindCaseSensitively);
        QVERIFY(symbol.isNull());

        symbol = headerDocument->findSymbol("saymessage");
        verifySymbol(headerDocument, symbol, "MyObject::sayMessage", Core::Symbol::Kind::Method, "sayMessage");

        symbol = headerDocument->findSymbol("m.message");
        QVERIFY(symbol.isNull());

        symbol = headerDocument->findSymbol("m.message", Core::TextDocument::FindRegexp);
        verifySymbol(headerDocument, symbol, "MyObject::m_message", Core::Symbol::Kind::Field, "m_message");
    }
};

QTEST_MAIN(TestCppDocument)
#include "tst_cppdocument.moc"
