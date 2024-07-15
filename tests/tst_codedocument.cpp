/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"
#include "core/codedocument.h"
#include "core/knutcore.h"
#include "core/lsp_utils.h"
#include "core/project.h"
#include "core/querymatch.h"

#include <QAction>
#include <QPlainTextEdit>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QTest>
#include <kdalgorithms.h>

#define INIT_KNUT_PROJECT                                                                                              \
    Core::KnutCore core;                                                                                               \
    auto project = Core::Project::instance();                                                                          \
    project->setRoot(Test::testDataPath() + "/projects/cpp-project")

class TestCodeDocument : public QObject
{
    Q_OBJECT

    void verifySymbol(Core::CodeDocument *document, const Core::Symbol *symbol, const QString &name,
                      Core::Symbol::Kind kind, const QString &selectionText)
    {
        QVERIFY(symbol != nullptr);
        QCOMPARE(symbol->name(), name);
        QCOMPARE(symbol->kind(), kind);
        document->selectRange(symbol->selectionRange());
        QCOMPARE(document->selectedText(), selectionText);
    }

    void verifySwitchDeclarationDefinition(Core::CodeDocument *sourcefile, Core::CodeDocument *targetfile, int line,
                                           const QString &selectedText)
    {
        auto result = qobject_cast<Core::CodeDocument *>(sourcefile->switchDeclarationDefinition());
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

        INIT_KNUT_PROJECT;

        auto cppDocument = qobject_cast<Core::CodeDocument *>(project->open("myobject.cpp"));
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

        auto headerDocument = qobject_cast<Core::CodeDocument *>(project->open("myobject.h"));

        const auto headerSymbols = headerDocument->symbols();
        QCOMPARE(headerSymbols.size(), 11);

        verifySymbol(headerDocument, headerSymbols.at(0), "MyObject", Core::Symbol::Kind::Class, "MyObject");
        verifySymbol(headerDocument, headerSymbols.at(1), "MyObject::MyObject", Core::Symbol::Kind::Constructor,
                     "MyObject");
        verifySymbol(headerDocument, headerSymbols.at(2), "MyObject::~MyObject", Core::Symbol::Kind::Constructor,
                     "MyObject");
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
        QTest::addColumn<QString>("symbolName");
        QTest::addColumn<Core::Symbol::Kind>("symbolKind");
        QTest::addColumn<Lsp::Position>("position");

        {
            QTest::newRow("main") << "main.cpp"
                                  << "main" << Core::Symbol::Kind::Function
                                  << Lsp::Position {.line = 6, .character = 6};
        }

        {
            QTest::newRow("main - freeFunction call")
                << "main.cpp"
                << "freeFunction" << Core::Symbol::Kind::Function << Lsp::Position {.line = 40, .character = 7};
        }

        {
            QTest::newRow("MyObject::m_message")
                << "myobject.h"
                << "MyObject::m_message" << Core::Symbol::Kind::Field << Lsp::Position {.line = 18, .character = 19};
        }
    }

    void symbolUnderCursor()
    {
        QFETCH(QString, fileName);
        QFETCH(QString, symbolName);
        QFETCH(Core::Symbol::Kind, symbolKind);
        QFETCH(Lsp::Position, position);

        INIT_KNUT_PROJECT;

        const auto document = qobject_cast<Core::CodeDocument *>(project->open(fileName));

        auto cursor = document->textEdit()->textCursor();
        cursor.setPosition(Core::Utils::lspToPos(*document, position));
        document->textEdit()->setTextCursor(cursor);

        auto actual = document->symbolUnderCursor();
        QVERIFY(actual);
        QCOMPARE(actual->name(), symbolName);
        QCOMPARE(actual->kind(), symbolKind);

        // Don't compare the actual to the symbols range. That might be too brittle when the test file changes.
        // Just check that the cursor is actually in the symbol range.
        QVERIFY(actual->selectionRange().contains(document->textEdit()->textCursor().position()));
        QVERIFY(actual->range().contains(document->textEdit()->textCursor().position()));
    }

    void symbolUnderCursorCache()
    {
        INIT_KNUT_PROJECT;

        const auto document = qobject_cast<Core::CodeDocument *>(project->open("main.cpp"));

        auto cursor = document->textEdit()->textCursor();
        cursor.setPosition(Core::Utils::lspToPos(*document, Lsp::Position {.line = 6 /*0-indexed*/, .character = 6}));
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

        INIT_KNUT_PROJECT;

        auto headerDocument = qobject_cast<Core::CodeDocument *>(project->open("myobject.h"));

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

        INIT_KNUT_PROJECT;

        auto codedocument = qobject_cast<Core::CodeDocument *>(project->open("main.cpp"));

        // Pre-open files, so clang has time to index them
        Core::Project::instance()->get("myobject.cpp");

        // Select the first use of the MyObject -> goTo declaration of the instance
        QVERIFY(codedocument->find("object.sayMessage()"));

        auto result = qobject_cast<Core::CodeDocument *>(codedocument->followSymbol());

        QCOMPARE(result, codedocument);
        QVERIFY(codedocument->hasSelection());
        auto cursor = codedocument->textEdit()->textCursor();
        QCOMPARE(cursor.blockNumber(), 7); // lines are 0-indexed, so 7 => line 8
        QCOMPARE(cursor.selectedText(), QString("object"));

        // select some empty piece of code -> don't do anything
        codedocument->gotoStartOfLine();
        cursor = codedocument->textEdit()->textCursor();
        QVERIFY(!codedocument->followSymbol());
        // The cursor should not change if followSymbol fails
        QCOMPARE(cursor, codedocument->textEdit()->textCursor());

        // Select a function call -> goto Function declaration
        QVERIFY(codedocument->find("sayMessage()"));
        result = qobject_cast<Core::CodeDocument *>(codedocument->followSymbol());
        QVERIFY(result);
        QVERIFY(result->fileName().endsWith("myobject.h"));
        cursor = result->textEdit()->textCursor();
        QCOMPARE(cursor.blockNumber(), 8); // lines are 0-indexed, so 8 => line 9
        QCOMPARE(cursor.selectedText(), QString("sayMessage"));

        // Selected a function declaration -> goTo function definition
        result = qobject_cast<Core::CodeDocument *>(result->followSymbol());
        QVERIFY(result);
        QVERIFY(result->fileName().endsWith("myobject.cpp"));
        cursor = result->textEdit()->textCursor();
        QCOMPARE(cursor.blockNumber(), 12); // lines are 0-indexed, so 12 => line 13
        QCOMPARE(cursor.selectedText(), QString("sayMessage"));

        // Selected a function definition -> goTo function declaration
        result = qobject_cast<Core::CodeDocument *>(result->followSymbol());
        QVERIFY(result);
        QVERIFY(result->fileName().endsWith("myobject.h"));
        cursor = result->textEdit()->textCursor();
        QCOMPARE(cursor.blockNumber(), 8); // lines are 0-indexed, so 9 => line 10
        QCOMPARE(cursor.selectedText(), QString("sayMessage"));
    }

    void switchDeclarationDefinition()
    {
        CHECK_CLANGD_VERSION;

        INIT_KNUT_PROJECT;

        auto mainfile = qobject_cast<Core::CodeDocument *>(project->open("main.cpp"));
        auto cppfile = qobject_cast<Core::CodeDocument *>(project->open("myobject.cpp"));
        auto headerfile = qobject_cast<Core::CodeDocument *>(project->open("myobject.h"));

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

        Test::FileTester file(Test::testDataPath() + "/tst_codedocument/notifyEditorChanges/section.cpp");
        {
            Core::KnutCore core;
            auto project = Core::Project::instance();
            project->setRoot(Test::testDataPath() + "/cpp-project");

            auto cppFile = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->get(file.fileName()));
            QVERIFY(cppFile);

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
        CHECK_CLANGD;

        INIT_KNUT_PROJECT;

        auto codedocument = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->get("myobject.h"));

        auto symbol = codedocument->findSymbol("MyObject");
        QVERIFY(symbol);

        QAction signalled;

        codedocument->hover(symbol->selectionRange().start + 1, [&signalled](const auto &) {
            signalled.trigger();
        });

        QSignalSpy spy(&signalled, &QAction::triggered);
        QVERIFY(spy.wait());
    }

    void query()
    {
        INIT_KNUT_PROJECT;

        auto codedocument = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->get("main.cpp"));

        Test::LogCounter counter;
        auto matches = codedocument->query(R"EOF(
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
        const auto &captures = match.captures();
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
        INIT_KNUT_PROJECT;

        auto codedocument = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->get("main.cpp"));

        Test::LogCounter counter;

        auto matches = codedocument->query(R"EOF(
            (function_definition
              declarator: (function_declarator
                declarator: (identifier) @name (eq? @name "non_existent_function")))
        )EOF");
        QVERIFY(matches.isEmpty());
        // Query is correct, but returns no result, so should produce no log output.
        QCOMPARE(counter.count(), 0);

        matches = codedocument->query("invalid query");
        QVERIFY(matches.isEmpty());
        // Invalid query, so should produce log output.
        QCOMPARE(counter.count(), 1);
    }

    void queryInRange()
    {
        INIT_KNUT_PROJECT;

        auto codedocument = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->get("main.cpp"));

        // Selection should include two function definitions "myFreeFunction" and "myOtherFreeFunction"
        codedocument->gotoLine(19);
        codedocument->selectNextLine(20);
        codedocument->selectEndOfLine();

        auto range = codedocument->createRangeMark();

        auto matches = codedocument->queryInRange(range, R"EOF(
                (function_definition)
                      )EOF");

        QCOMPARE(matches.size(), 2);
    }

    void ast()
    {
        Test::FileTester header(Test::testDataPath() + "/tst_codedocument/ast/header.h");

        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/tst_codedocument/ast/");

        auto document = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->get(header.fileName()));
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

    void selectLargerSyntaxNode()
    {
        INIT_KNUT_PROJECT;

        auto codedocument = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->get("myobject.cpp"));

        QVERIFY(codedocument->find("string&"));
        QCOMPARE(codedocument->selectedText(), "string&");

        auto result = codedocument->selectLargerSyntaxNode();
        QCOMPARE(result, codedocument->position());
        QCOMPARE(codedocument->selectedText(), "const std::string& message");

        result = codedocument->selectLargerSyntaxNode();
        QCOMPARE(result, codedocument->position());
        QCOMPARE(codedocument->selectedText(), "(const std::string& message)");

        result = codedocument->selectLargerSyntaxNode(2);
        QCOMPARE(result, codedocument->position());
        QCOMPARE(codedocument->selectedText(),
                 "MyObject::MyObject(const std::string& message)\n"
                 "    : m_message(message)\n"
                 "{}");
    }

    // Regression test to ensure that certain unnamed nodes are handled correctly.
    // For example the `const` qualifier has a `type_qualifier` in the AST, which covers the same span.
    // Make sure that selectLargerSyntaxNode actually goes up far enough to actually expand the selection.
    void selectLargerSyntaxNode_unnamed_nodes()
    {
        INIT_KNUT_PROJECT;

        auto codedocument = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->get("myobject.cpp"));

        QVERIFY(codedocument->find("const"));
        QCOMPARE(codedocument->selectedText(), "const");

        auto result = codedocument->selectLargerSyntaxNode();
        QCOMPARE(result, codedocument->position());
        QCOMPARE(codedocument->selectedText(), "const std::string& message");
    }

    void selectSmallerSyntaxNode()
    {
        INIT_KNUT_PROJECT;

        auto codedocument = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->get("myobject.cpp"));
        QVERIFY(codedocument->find("(const std::string& message)"));

        // This should select the first **named** child, so skip the opening "("
        auto result = codedocument->selectSmallerSyntaxNode();
        QCOMPARE(result, codedocument->position());
        QCOMPARE(codedocument->selectedText(), "const std::string& message");

        result = codedocument->selectSmallerSyntaxNode();
        QCOMPARE(result, codedocument->position());
        QCOMPARE(codedocument->selectedText(), "const");

        codedocument->selectLargerSyntaxNode(2);
        QCOMPARE(codedocument->selectedText(), "(const std::string& message)");

        result = codedocument->selectSmallerSyntaxNode(2);
        QCOMPARE(result, codedocument->position());
        QCOMPARE(codedocument->selectedText(), "const");
    }

    void selectNextSyntaxNode()
    {
        INIT_KNUT_PROJECT;

        auto codedocument = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->get("main.cpp"));

        // Test that a partial selection searches from the next larger syntax node.
        QVERIFY(codedocument->find("gned int"));

        auto result = codedocument->selectNextSyntaxNode();
        QCOMPARE(result, codedocument->position());
        QCOMPARE(codedocument->selectedText(), "long long");

        // Skipping past the end of a node should select the next sibling from the parent.
        result = codedocument->selectNextSyntaxNode(4);
        QCOMPARE(result, codedocument->position());
        QCOMPARE(codedocument->selectedText(), "{\n    return \"hello\";\n}");

        auto lastFunction = "int freeFunction(unsigned, long long)\n"
                            "{\n"
                            "        return 5;\n"
                            "}";
        result = codedocument->selectNextSyntaxNode(2);
        QCOMPARE(result, codedocument->position());
        QCOMPARE(codedocument->selectedText(), lastFunction);

        // No matter how far we go past the end, the selection should not change
        QCOMPARE(codedocument->selectNextSyntaxNode(10), result);
        QCOMPARE(codedocument->selectedText(), lastFunction);
    }

    void selectPreviousSyntaxNode()
    {
        INIT_KNUT_PROJECT;

        auto codedocument = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->get("main.cpp"));

        // Test that a partial selection searches from the next larger syntax node.
        QVERIFY(codedocument->find("ction(1,"));

        auto result = codedocument->selectPreviousSyntaxNode();
        QCOMPARE(codedocument->position(), result);
        QCOMPARE(codedocument->selectedText(), "object.sayMessage(\"Another message\" /*a comment*/);");

        // Skipping past the end of a node should select the previous sibling from the parent.
        result = codedocument->selectPreviousSyntaxNode(3);
        QCOMPARE(codedocument->position(), result);
        QCOMPARE(codedocument->selectedText(), "main(int argc, char *argv[])");

        // going past the end selects the last sibling
        result = codedocument->selectPreviousSyntaxNode(10);
        QCOMPARE(codedocument->position(), result);
        QCOMPARE(codedocument->selectedText(), "#include <iostream>\n");

        QCOMPARE(codedocument->selectPreviousSyntaxNode(10), result);
        QCOMPARE(codedocument->selectedText(), "#include <iostream>\n");
    }
};

QTEST_MAIN(TestCodeDocument)
#include "tst_codedocument.moc"
