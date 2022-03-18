#include "core/knutcore.h"
#include "core/lspdocument.h"
#include "core/project.h"

#include <QPlainTextEdit>
#include <QTest>
#include <qtestcase.h>

#include "common/test_utils.h"

class TestLspDocument : public QObject
{
    Q_OBJECT

    void verifySymbol(Core::LspDocument *document, const Core::Symbol &symbol, const QString &name,
                      Core::Symbol::Kind kind, const QString &selectionText)
    {
        QVERIFY(!symbol.isNull());
        QCOMPARE(symbol.name, name);
        QCOMPARE(symbol.kind, kind);
        document->selectRange(symbol.selectionRange);
        QCOMPARE(document->selectedText(), selectionText);
    }

    void verifySwitchDeclarationDefinition(Core::LspDocument *sourcefile, Core::LspDocument *targetfile, int line,
                                           const QString &selectedText)
    {
        auto result = dynamic_cast<Core::LspDocument *>(sourcefile->switchDeclarationDefinition());
        QVERIFY(result);
        QCOMPARE(result, targetfile);
        auto cursor = result->textEdit()->textCursor();
        QCOMPARE(cursor.blockNumber(), line - 1);
        QCOMPARE(cursor.selectedText(), selectedText);
    }

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void symbols()
    {
        CHECK_CLANGD_VERSION;

        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/cpp-project");

        auto cppDocument = qobject_cast<Core::LspDocument *>(project->open("myobject.cpp"));
        const auto cppSymbols = cppDocument->symbols();
        QCOMPARE(cppSymbols.size(), 3);

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

        auto headerDocument = qobject_cast<Core::LspDocument *>(project->open("myobject.h"));

        const auto headerSymbols = headerDocument->symbols();
        QCOMPARE(headerSymbols.size(), 5);

        verifySymbol(headerDocument, headerSymbols.at(0), "MyObject", Core::Symbol::Kind::Class, "MyObject");
        verifySymbol(headerDocument, headerSymbols.at(1), "MyObject::MyObject", Core::Symbol::Kind::Constructor,
                     "MyObject");
        verifySymbol(headerDocument, headerSymbols.at(2), "MyObject::~MyObject", Core::Symbol::Kind::Constructor, "~");
        verifySymbol(headerDocument, headerSymbols.at(3), "MyObject::sayMessage", Core::Symbol::Kind::Method,
                     "sayMessage");
        verifySymbol(headerDocument, headerSymbols.at(4), "MyObject::m_message", Core::Symbol::Kind::Field,
                     "m_message");
    }

    void findSymbol()
    {
        CHECK_CLANGD_VERSION;

        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/cpp-project");

        auto headerDocument = qobject_cast<Core::LspDocument *>(project->open("myobject.h"));

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

    void followSymbol()
    {
        CHECK_CLANGD_VERSION;

        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/cpp-project/");
        auto lspdocument = qobject_cast<Core::LspDocument *>(Core::Project::instance()->open("main.cpp"));

        // Pre-open files, so clang has time to index them
        Core::Project::instance()->get("myobject.cpp");

        // Select the first use of the MyObject -> goTo declaration of the instance
        QVERIFY(lspdocument->find("object.sayMessage()"));

        auto result = dynamic_cast<Core::LspDocument *>(lspdocument->followSymbol());

        QCOMPARE(result, lspdocument);
        QVERIFY(lspdocument->hasSelection());
        auto cursor = lspdocument->textEdit()->textCursor();
        QCOMPARE(cursor.blockNumber(), 5); // lines are 0-indexed, so 5 => line 6
        QCOMPARE(cursor.selectedText(), QString("object"));

        // select some empty piece of code -> don't do anything
        lspdocument->gotoStartOfLine();
        cursor = lspdocument->textEdit()->textCursor();
        QVERIFY(!lspdocument->followSymbol());
        // The cursor should not change if followSymbol fails
        QCOMPARE(cursor, lspdocument->textEdit()->textCursor());

        // Select a function call -> goto Function declaration
        QVERIFY(lspdocument->find("sayMessage()"));
        result = dynamic_cast<Core::LspDocument *>(lspdocument->followSymbol());
        QVERIFY(result);
        QVERIFY(result->fileName().endsWith("myobject.h"));
        cursor = result->textEdit()->textCursor();
        QCOMPARE(cursor.blockNumber(), 8); // lines are 0-indexed, so 8 => line 9
        QCOMPARE(cursor.selectedText(), QString("sayMessage"));

        // Selected a function declaration -> goTo function definition
        result = dynamic_cast<Core::LspDocument *>(result->followSymbol());
        QVERIFY(result);
        QVERIFY(result->fileName().endsWith("myobject.cpp"));
        cursor = result->textEdit()->textCursor();
        QCOMPARE(cursor.blockNumber(), 12); // lines are 0-indexed, so 12 => line 13
        QCOMPARE(cursor.selectedText(), QString("sayMessage"));

        // Selected a function definition -> goTo function declaration
        result = dynamic_cast<Core::LspDocument *>(result->followSymbol());
        QVERIFY(result);
        QVERIFY(result->fileName().endsWith("myobject.h"));
        cursor = result->textEdit()->textCursor();
        QCOMPARE(cursor.blockNumber(), 8); // lines are 0-indexed, so 9 => line 10
        QCOMPARE(cursor.selectedText(), QString("sayMessage"));
    }

    void switchDeclarationDefinition()
    {
        CHECK_CLANGD_VERSION;

        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/cpp-project/");
        auto mainfile = qobject_cast<Core::LspDocument *>(project->open("main.cpp"));
        auto cppfile = qobject_cast<Core::LspDocument *>(project->open("myobject.cpp"));
        auto headerfile = qobject_cast<Core::LspDocument *>(project->open("myobject.h"));

        // Cursor outside of a function - do nothing
        mainfile->gotoStartOfDocument();
        auto oldcursor = mainfile->textEdit()->textCursor();
        QVERIFY(!mainfile->switchDeclarationDefinition());
        QCOMPARE(mainfile->textEdit()->textCursor(), oldcursor);

        // Cursor within a function without declaration -> Select definition
        QVERIFY(mainfile->find("object.sayMessage()"));
        verifySwitchDeclarationDefinition(mainfile, mainfile, 5, "main");

        // Cursor within function definition - select Declaration
        QVERIFY(cppfile->find("cout"));
        verifySwitchDeclarationDefinition(cppfile, headerfile, 9, "sayMessage");

        // Cursor at function declaration - select Definition
        verifySwitchDeclarationDefinition(headerfile, cppfile, 13, "sayMessage");

        // Cursor at constructor definition - select Declaration
        cppfile->gotoStartOfDocument();
        QVERIFY(cppfile->find("MyObject::MyObject"));
        verifySwitchDeclarationDefinition(cppfile, headerfile, 7, "MyObject");

        // Cursor at constructor declaration - select Definition
        verifySwitchDeclarationDefinition(headerfile, cppfile, 6, "MyObject");
    }
};

QTEST_MAIN(TestLspDocument)
#include "tst_lspdocument.moc"
