#include "core/knutcore.h"
#include "core/lspdocument.h"
#include "core/project.h"
#include "core/querymatch.h"

#include <QAction>
#include <QPlainTextEdit>
#include <QSignalSpy>
#include <QTest>
#include <qtestcase.h>
#include <QTemporaryFile>

#include <kdalgorithms.h>

#include "common/test_utils.h"

class TestLspDocument : public QObject
{
    Q_OBJECT

    void verifySymbol(Core::LspDocument *document, const Core::Symbol *symbol, const QString &name,
                      Core::Symbol::Kind kind, const QString &selectionText)
    {
        QVERIFY(symbol != nullptr);
        QCOMPARE(symbol->name(), name);
        QCOMPARE(symbol->kind(), kind);
        document->selectRange(symbol->selectionRange());
        QCOMPARE(document->selectedText(), selectionText);
    }

    void verifySwitchDeclarationDefinition(Core::LspDocument *sourcefile, Core::LspDocument *targetfile, int line,
                                           const QString &selectedText)
    {
        auto result = qobject_cast<Core::LspDocument *>(sourcefile->switchDeclarationDefinition());
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
        project->setRoot(Test::testDataPath() + "/projects/cpp-project");

        auto cppDocument = qobject_cast<Core::LspDocument *>(project->open("myobject.cpp"));
        const auto cppSymbols = cppDocument->symbols();
        QCOMPARE(cppSymbols.size(), 4);

        auto constructor = cppSymbols.first();
        verifySymbol(cppDocument, constructor, "MyObject::MyObject", Core::Symbol::Kind::Constructor, "MyObject");
        cppDocument->selectRange(constructor->range());
        QCOMPARE(cppDocument->selectedText(),
                 QString(
                     R"(MyObject::MyObject(const std::string& message)
    : m_message(message)
{})"));

        auto function = cppSymbols.last();
        verifySymbol(cppDocument, function, "MyObject::sayMessage", Core::Symbol::Kind::Method, "sayMessage");
        cppDocument->selectRange(function->range());
        QCOMPARE(cppDocument->selectedText(),
                 QString(
                     R"(void MyObject::sayMessage(const std::string& test) {
    m_enum = MyEnum::C;
    std::cout << test << std::endl;
})"));

        auto headerDocument = qobject_cast<Core::LspDocument *>(project->open("myobject.h"));

        const auto headerSymbols = headerDocument->symbols();
        QCOMPARE(headerSymbols.size(), 11);

        verifySymbol(headerDocument, headerSymbols.at(0), "MyObject", Core::Symbol::Kind::Class, "MyObject");
        verifySymbol(headerDocument, headerSymbols.at(1), "MyObject::MyObject", Core::Symbol::Kind::Constructor,
                     "MyObject");
        verifySymbol(headerDocument, headerSymbols.at(2), "MyObject::~MyObject", Core::Symbol::Kind::Constructor, "~");
        verifySymbol(headerDocument, headerSymbols.at(3), "MyObject::sayMessage", Core::Symbol::Kind::Method,
                     "sayMessage");
        verifySymbol(headerDocument, headerSymbols.at(4), "MyObject::sayMessage", Core::Symbol::Kind::Method,
                     "sayMessage");
        verifySymbol(headerDocument, headerSymbols.at(5), "MyObject::MyEnum", Core::Symbol::Kind::Enum, "MyEnum");
        verifySymbol(headerDocument, headerSymbols.at(6), "MyObject::MyEnum::A", Core::Symbol::Kind::Enum, "A");
        verifySymbol(headerDocument, headerSymbols.at(7), "MyObject::MyEnum::B", Core::Symbol::Kind::Enum, "B");
        verifySymbol(headerDocument, headerSymbols.at(8), "MyObject::MyEnum::C", Core::Symbol::Kind::Enum, "C");
        verifySymbol(headerDocument, headerSymbols.at(9), "MyObject::m_message", Core::Symbol::Kind::Field,
                     "m_message");
        verifySymbol(headerDocument, headerSymbols.at(10), "MyObject::m_enum", Core::Symbol::Kind::Field, "m_enum");
    }

    void symbolUnderCursor_data()
    {
        QTest::addColumn<QString>("fileName");
        QTest::addColumn<Core::Symbol *>("symbol");
        QTest::addColumn<Lsp::Position>("position");

        {
            auto symbol = Core::Symbol::makeSymbol(this, "main", "int (int, char **)", "", Core::Symbol::Kind::Function,
                                                   Core::TextRange {.start = 0, .end = 1},
                                                   Core::TextRange {.start = 10, .end = 11});
            QTest::newRow("main") << "main.cpp" << symbol << Lsp::Position {.line = 6, .character = 6};
        }

        {
            auto symbol = Core::Symbol::makeSymbol(this, "argc", "int", "", Core::Symbol::Kind::Variable,
                                                   Core::TextRange {.start = 0, .end = 1},
                                                   Core::TextRange {.start = 10, .end = 11});
            QTest::newRow("main - argc") << "main.cpp" << symbol << Lsp::Position {.line = 6, .character = 16};
        }

        {
            auto symbol = Core::Symbol::makeSymbol(this, "argv", "char **", "", Core::Symbol::Kind::Variable,
                                                   Core::TextRange {.start = 0, .end = 1},
                                                   Core::TextRange {.start = 10, .end = 11});
            QTest::newRow("main - argc") << "main.cpp" << symbol << Lsp::Position {.line = 6, .character = 27};
        }

        {
            auto symbol = Core::Symbol::makeSymbol(this, "object", "class MyObject", "\"myobject.h\"",
                                                   Core::Symbol::Kind::Variable, Core::TextRange {.start = 0, .end = 1},
                                                   Core::TextRange {.start = 10, .end = 11});
            QTest::newRow("main - object variable")
                << "main.cpp" << symbol << Lsp::Position {.line = 7, .character = 16};
        }

        {
            auto symbol = Core::Symbol::makeSymbol(this, "sayMessage", "void ()", "\"myobject.h\"",
                                                   Core::Symbol::Kind::Method, Core::TextRange {.start = 0, .end = 1},
                                                   Core::TextRange {.start = 10, .end = 11});
            QTest::newRow("main - object.sayMessage()")
                << "main.cpp" << symbol << Lsp::Position {.line = 9, .character = 14};
        }

        {
            auto symbol = Core::Symbol::makeSymbol(this, "freeFunction", "int (unsigned int, long long)", "",
                                                   Core::Symbol::Kind::Function, Core::TextRange {.start = 0, .end = 1},
                                                   Core::TextRange {.start = 10, .end = 11});
            QTest::newRow("freeFunction - declaration")
                << "main.cpp" << symbol << Lsp::Position {.line = 4, .character = 7};
        }

        {
            auto symbol = Core::Symbol::makeSymbol(this, "freeFunction", "int (unsigned int, long long)", "",
                                                   Core::Symbol::Kind::Function, Core::TextRange {.start = 0, .end = 1},
                                                   Core::TextRange {.start = 10, .end = 11});
            QTest::newRow("main - freeFunction call")
                << "main.cpp" << symbol << Lsp::Position {.line = 11, .character = 7};
        }

        {
            auto symbol = Core::Symbol::makeSymbol(this, "cout", "std::ostream", "<iostream>",
                                                   Core::Symbol::Kind::Variable, Core::TextRange {.start = 0, .end = 1},
                                                   Core::TextRange {.start = 10, .end = 11});
            QTest::newRow("MyObject::sayMessage - cout")
                << "myobject.cpp" << symbol << Lsp::Position {.line = 13, .character = 10};
        }

        {
            auto symbol = Core::Symbol::makeSymbol(
                this, "std", "namespace std {\n}", "<iostream>", Core::Symbol::Kind::Namespace,
                Core::TextRange {.start = 0, .end = 1}, Core::TextRange {.start = 10, .end = 11});
            QTest::newRow("MyObject::sayMessage - cout")
                << "myobject.cpp" << symbol << Lsp::Position {.line = 13, .character = 7};
        }

        {
            auto symbol = Core::Symbol::makeSymbol(this, "MyObject::m_message", "std::string", "\"myobject.h\"",
                                                   Core::Symbol::Kind::Field, Core::TextRange {.start = 0, .end = 1},
                                                   Core::TextRange {.start = 10, .end = 11});
            QTest::newRow("MyObject::m_message")
                << "myobject.h" << symbol << Lsp::Position {.line = 18, .character = 19};
        }

        {
            auto symbol = Core::Symbol::makeSymbol(this, "m_enum", "enum MyObject::MyEnum", "\"myobject.h\"",
                                                   Core::Symbol::Kind::Field, Core::TextRange {.start = 0, .end = 1},
                                                   Core::TextRange {.start = 10, .end = 11});
            QTest::newRow("MyObject::sayMessage - m_enum")
                << "myobject.cpp" << symbol << Lsp::Position {.line = 17, .character = 7};
        }

        {
            auto symbol = Core::Symbol::makeSymbol(
                this, "C", "Type: enum MyObject::MyEnum\nValue = 3\n\n// In MyObject::MyEnum\npublic: C = A | B",
                "\"myobject.h\"", Core::Symbol::Kind::Enum, Core::TextRange {.start = 0, .end = 1},
                Core::TextRange {.start = 10, .end = 11});
            QTest::newRow("MyObject::sayMessage - MyEnum::C")
                << "myobject.cpp" << symbol << Lsp::Position {.line = 17, .character = 22};
        }

        {
            auto symbol = Core::Symbol::makeSymbol(this, "MyObject", "class MyObject {}", "\"myobject.h\"",
                                                   Core::Symbol::Kind::Class, Core::TextRange {.start = 0, .end = 1},
                                                   Core::TextRange {.start = 10, .end = 11});
            QTest::newRow("main -  class MyObject")
                << "main.cpp" << symbol << Lsp::Position {.line = 7, .character = 7};
        }
    }

    void symbolUnderCursor()
    {
        QFETCH(QString, fileName);
        QFETCH(Core::Symbol *, symbol);
        QFETCH(Lsp::Position, position);

        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/projects/cpp-project");

        const auto document = qobject_cast<Core::LspDocument *>(project->open(fileName));

        auto cursor = document->textEdit()->textCursor();
        cursor.setPosition(document->toPos(position));
        document->textEdit()->setTextCursor(cursor);

        auto actual = document->symbolUnderCursor();
        QVERIFY(actual);
        QCOMPARE(actual->name(), symbol->name());
        // Some version of clang gives the full type "enum MyObject::MyEnum", other not "MyEnum"
        QVERIFY(symbol->description().contains(actual->description()));
        QCOMPARE(actual->kind(), symbol->kind());
        // clangd 17 provides an import location, if it exists, check that it's correct!
        if (!actual->importLocation().isEmpty()) {
            QCOMPARE(actual->importLocation(), symbol->importLocation());
        }

        // Don't compare the actual to the symbols range. That might be too brittle when the test file changes.
        // Just check that the cursor is actually in the symbol range.
        QVERIFY(actual->selectionRange().contains(document->textEdit()->textCursor().position()));
        QVERIFY(actual->range().contains(document->textEdit()->textCursor().position()));
    }

    void symbolUnderCursorCache()
    {
        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/projects/cpp-project");

        const auto document = qobject_cast<Core::LspDocument *>(project->open("main.cpp"));

        auto cursor = document->textEdit()->textCursor();
        cursor.setPosition(document->toPos(Lsp::Position {.line = 7, .character = 16}));
        document->textEdit()->setTextCursor(cursor);

        auto symbol1 = document->symbolUnderCursor();
        QVERIFY(symbol1);

        auto symbol2 = document->symbolUnderCursor();
        QVERIFY(symbol2);
        QVERIFY2(symbol1 == symbol2, "Asking for the same symbol under cursor twice should return the same pointer.");
    }

    void findSymbol()
    {
        CHECK_CLANGD_VERSION;

        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/projects/cpp-project");

        auto headerDocument = qobject_cast<Core::LspDocument *>(project->open("myobject.h"));

        auto symbol = headerDocument->findSymbol("MyObject", Core::TextDocument::FindWholeWords);
        verifySymbol(headerDocument, symbol, "MyObject", Core::Symbol::Kind::Class, "MyObject");

        symbol = headerDocument->findSymbol("m_message", Core::TextDocument::FindWholeWords);
        QVERIFY(symbol == nullptr);

        symbol = headerDocument->findSymbol("m_message");
        verifySymbol(headerDocument, symbol, "MyObject::m_message", Core::Symbol::Kind::Field, "m_message");

        symbol = headerDocument->findSymbol("saymessage", Core::TextDocument::FindCaseSensitively);
        QVERIFY(symbol == nullptr);

        symbol = headerDocument->findSymbol("saymessage");
        verifySymbol(headerDocument, symbol, "MyObject::sayMessage", Core::Symbol::Kind::Method, "sayMessage");

        symbol = headerDocument->findSymbol("m.message");
        QVERIFY(symbol == nullptr);

        symbol = headerDocument->findSymbol("m.message", Core::TextDocument::FindRegexp);
        verifySymbol(headerDocument, symbol, "MyObject::m_message", Core::Symbol::Kind::Field, "m_message");
    }

    void followSymbol()
    {
        CHECK_CLANGD_VERSION;

        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/projects/cpp-project");

        auto lspdocument = qobject_cast<Core::LspDocument *>(project->open("main.cpp"));

        // Pre-open files, so clang has time to index them
        Core::Project::instance()->get("myobject.cpp");

        // Select the first use of the MyObject -> goTo declaration of the instance
        QVERIFY(lspdocument->find("object.sayMessage()"));

        auto result = qobject_cast<Core::LspDocument *>(lspdocument->followSymbol());

        QCOMPARE(result, lspdocument);
        QVERIFY(lspdocument->hasSelection());
        auto cursor = lspdocument->textEdit()->textCursor();
        QCOMPARE(cursor.blockNumber(), 7); // lines are 0-indexed, so 7 => line 8
        QCOMPARE(cursor.selectedText(), QString("object"));

        // select some empty piece of code -> don't do anything
        lspdocument->gotoStartOfLine();
        cursor = lspdocument->textEdit()->textCursor();
        QVERIFY(!lspdocument->followSymbol());
        // The cursor should not change if followSymbol fails
        QCOMPARE(cursor, lspdocument->textEdit()->textCursor());

        // Select a function call -> goto Function declaration
        QVERIFY(lspdocument->find("sayMessage()"));
        result = qobject_cast<Core::LspDocument *>(lspdocument->followSymbol());
        QVERIFY(result);
        QVERIFY(result->fileName().endsWith("myobject.h"));
        cursor = result->textEdit()->textCursor();
        QCOMPARE(cursor.blockNumber(), 8); // lines are 0-indexed, so 8 => line 9
        QCOMPARE(cursor.selectedText(), QString("sayMessage"));

        // Selected a function declaration -> goTo function definition
        result = qobject_cast<Core::LspDocument *>(result->followSymbol());
        QVERIFY(result);
        QVERIFY(result->fileName().endsWith("myobject.cpp"));
        cursor = result->textEdit()->textCursor();
        QCOMPARE(cursor.blockNumber(), 12); // lines are 0-indexed, so 12 => line 13
        QCOMPARE(cursor.selectedText(), QString("sayMessage"));

        // Selected a function definition -> goTo function declaration
        result = qobject_cast<Core::LspDocument *>(result->followSymbol());
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
        project->setRoot(Test::testDataPath() + "/projects/cpp-project");

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
        verifySwitchDeclarationDefinition(mainfile, mainfile, 7, "main");

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

    void notifyEditorChanges()
    {
        // Regression test for KNUT-42 - LSP doesn't get notified of changes in Editor
        CHECK_CLANGD_VERSION;

        Test::FileTester file(Test::testDataPath() + "/tst_lspdocument/notifyEditorChanges/section.cpp");
        {
            Core::KnutCore core;
            auto project = Core::Project::instance();
            project->setRoot(Test::testDataPath() + "/cpp-project");

            auto cppFile = qobject_cast<Core::LspDocument *>(Core::Project::instance()->get(file.fileName()));

            const auto before = cppFile->findSymbol("Section::bar")->range();
            QCOMPARE(before.start, 326);
            QCOMPARE(before.end, 386);

            // Changing the code will cause the Language server to become out of sync with the
            // state in Knut. Therefore the next language server call will fail.
            cppFile->gotoStartOfDocument();
            cppFile->insert("\n");
            const auto after = cppFile->findSymbol("Section::bar")->range();
            QCOMPARE(after.start, 327);
            QCOMPARE(after.end, 387);
        }
    }

    void asyncHover()
    {
        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/projects/cpp-project");

        auto lspdocument = qobject_cast<Core::LspDocument *>(Core::Project::instance()->get("myobject.h"));

        auto symbol = lspdocument->findSymbol("MyObject");
        QVERIFY(symbol);

        QAction signalled;

        lspdocument->hover(symbol->selectionRange().start + 1, [&signalled](const auto &) {
            signalled.trigger();
        });

        QSignalSpy spy(&signalled, &QAction::triggered);
        QVERIFY(spy.wait());
    }

    void query()
    {
        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/projects/cpp-project");

        auto lspdocument = qobject_cast<Core::LspDocument *>(Core::Project::instance()->get("main.cpp"));

        Test::LogCounter counter;
        auto matches = lspdocument->query(R"EOF(
                (function_definition
                  type: (_) @return-type
                  declarator: (function_declarator
                    declarator: (identifier) @name (eq? @name "main")
                    parameters: (parameter_list
                      . ((parameter_declaration) @param ","?)*)))
                      )EOF");

        QCOMPARE(counter.count(), 0);

        QCOMPARE(matches.size(), 1);
        auto match = matches[0];
        auto captures = match.captures();
        QCOMPARE(captures.size(), 4);
        QCOMPARE(match.getAll("name").size(), 1);
        QCOMPARE(match.getAll("return-type").size(), 1);
        QCOMPARE(match.getAll("param").size(), 2);

        QCOMPARE(match.getAll("return-type").at(0).text(), "int");
        QCOMPARE(match.getAll("param").at(0).text(), "int argc");
        QCOMPARE(match.getAll("param").at(1).text(), "char *argv[]");
    }

    void failedQuery()
    {
        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/projects/cpp-project");

        auto lspdocument = qobject_cast<Core::LspDocument *>(Core::Project::instance()->get("main.cpp"));

        Test::LogCounter counter;

        auto matches = lspdocument->query(R"EOF(
            (function_definition
              declarator: (function_declarator
                declarator: (identifier) @name (eq? @name "non_existent_function")))
        )EOF");
        QVERIFY(matches.isEmpty());
        // Query is correct, but returns no result, so should produce no log output.
        QCOMPARE(counter.count(), 0);

        matches = lspdocument->query("invalid query");
        QVERIFY(matches.isEmpty());
        // Invalid query, so should produce log output.
        QCOMPARE(counter.count(), 1);
    }

    void queryInRange()
    {
        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/projects/cpp-project");

        auto lspdocument = qobject_cast<Core::LspDocument *>(Core::Project::instance()->get("main.cpp"));

        // Selection should include two function definitions "myFreeFunction" and "myOtherFreeFunction"
        lspdocument->gotoLine(19);
        lspdocument->selectNextLine(20);
        lspdocument->selectEndOfLine();

        auto range = lspdocument->createRangeMark();

        auto matches = lspdocument->queryInRange(range, R"EOF(
                (function_definition)
                      )EOF");

        QCOMPARE(matches.size(), 2);
    }

    void ast()
    {
        Test::FileTester header(Test::testDataPath() + "/tst_lspdocument/ast/header.h");

        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/tst_lspdocument/ast/");

        auto document = qobject_cast<Core::LspDocument *>(Core::Project::instance()->get(header.fileName()));
        QVERIFY(document);

        document->gotoLine(6, 9);
        auto foo = document->astNodeAt(document->position());
        QVERIFY(foo.isValid());
        QCOMPARE(foo.type(), "function_definition");
        QCOMPARE(foo.startPos(), 38);
        QCOMPARE(foo.endPos(), 92);

        {
            auto parent = foo.parentNode();
            QVERIFY(parent.isValid());
            QCOMPARE(parent.type(), "field_declaration_list");
        }

        auto children = foo.childrenNodes();
        QCOMPARE(children.size(), 3);

        QCOMPARE(children[0].text(), "void");
        QCOMPARE(children[0].type(), "primitive_type");
        QCOMPARE(children[0].startPos(), 38);
        QCOMPARE(children[0].endPos(), 42);

        QVERIFY(foo.isValid());

        // Change text before node, position etc should adopt
        document->gotoLine(1);
        document->insert("#include <test.h>\n\n");

        QVERIFY(foo.isValid());

        QCOMPARE(foo.type(), "function_definition");
        QCOMPARE(foo.startPos(), 57);
        QCOMPARE(foo.endPos(), 111);

        {
            auto parent = foo.parentNode();
            QVERIFY(parent.isValid());
            QCOMPARE(parent.type(), "field_declaration_list");
        }

        // Change text after node, position etc shouldn't change
        document->gotoEndOfDocument();
        document->insert("void bar();\n");

        QVERIFY(foo.isValid());

        QCOMPARE(foo.type(), "function_definition");
        QCOMPARE(foo.startPos(), 57);
        QCOMPARE(foo.endPos(), 111);
    }

};

QTEST_MAIN(TestLspDocument)
#include "tst_lspdocument.moc"
