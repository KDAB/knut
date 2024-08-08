/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_cpputils.h"
#include "common/test_utils.h"
#include "core/cppdocument.h"
#include "core/knutcore.h"
#include "core/utils.h"

#include <QFileInfo>
#include <QPlainTextEdit>

class TestCppDocument : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void correspondingHeaderSource_data()
    {
        QTest::addColumn<QString>("headerOrSource");
        QTest::addColumn<QString>("sourceOrHeader");

        QTest::newRow("cpp") << "test/hello.cpp"
                             << "test/hello.h";
        QTest::newRow("h") << "test/hello.h"
                           << "test/hello.cpp";
        QTest::newRow("cxx") << "test/world.cxx"
                             << "test/world.hpp";
        QTest::newRow("folder") << "folder1/foo.cpp"
                                << "folder2/foo.h";
        QTest::newRow("subfolder") << "test/subfolder1/foo.cpp"
                                   << "test/subfolder2/foo.h";
        QTest::newRow("notexist") << "test/bar.cpp"
                                  << "";
    }

    void correspondingHeaderSource()
    {
        QFETCH(QString, headerOrSource);
        QFETCH(QString, sourceOrHeader);

        sourceOrHeader =
            sourceOrHeader.isEmpty() ? "" : Test::testDataPath() + "/tst_cppdocument/headerSource/" + sourceOrHeader;

        Test::testCppDocument("/tst_cppdocument/headerSource/", headerOrSource,
                              [&sourceOrHeader](Core::CppDocument *document) {
                                  QCOMPARE(sourceOrHeader, document->correspondingHeaderSource());
                              });
    }

    void insertForwardDeclaration()
    {
        {
            Test::FileTester file(Test::testDataPath() + "/tst_cppdocument/forwardDeclaration/header.h");

            Test::testCppDocument("/tst_cppdocument/forwardDeclaration", file.fileName(), [](auto *headerFile) {
                QCOMPARE(headerFile->insertForwardDeclaration("class Foo"), true);
                QCOMPARE(headerFile->insertForwardDeclaration("class Foo::Bar::FooBar"), true);
                QCOMPARE(headerFile->insertForwardDeclaration("class Foo::Bar::FooBar"), false);
                QCOMPARE(headerFile->insertForwardDeclaration(" "), false);
                QCOMPARE(headerFile->insertForwardDeclaration("Foo::Bar::FooBar"), false);
                QCOMPARE(headerFile->insertForwardDeclaration("struct Neo"), true);
                QCOMPARE(headerFile->insertForwardDeclaration("struct SFoo::SBar::Uno"), true);
                headerFile->save();
            });
            QVERIFY(file.compare());
        }
        {
            Test::FileTester file(Test::testDataPath() + "/tst_cppdocument/forwardDeclaration/header-pragma.h");
            Test::testCppDocument("/tst_cppdocument/forwardDeclaration", file.fileName(), [](auto *headerFile) {
                QCOMPARE(headerFile->insertForwardDeclaration("class Foo"), true);
                QCOMPARE(headerFile->insertForwardDeclaration("class Foo::Bar::FooBar"), true);
                QCOMPARE(headerFile->insertForwardDeclaration("class Foo::Bar::FooBar"), false);
                QCOMPARE(headerFile->insertForwardDeclaration(" "), false);
                QCOMPARE(headerFile->insertForwardDeclaration("Foo::Bar::FooBar"), false);
                QCOMPARE(headerFile->insertForwardDeclaration("struct Neo"), true);
                QCOMPARE(headerFile->insertForwardDeclaration("struct SFoo::SBar::Uno"), true);
                headerFile->save();
            });
            QVERIFY(file.compare());
        }
        {
            Test::FileTester file(Test::testDataPath() + "/tst_cppdocument/forwardDeclaration/header-guard.h");
            Test::testCppDocument("/tst_cppdocument/forwardDeclaration", file.fileName(), [](auto *headerFile) {
                QCOMPARE(headerFile->insertForwardDeclaration("class Foo"), true);
                QCOMPARE(headerFile->insertForwardDeclaration("class Foo::Bar::FooBar"), true);
                QCOMPARE(headerFile->insertForwardDeclaration("class Foo::Bar::FooBar"), false);
                QCOMPARE(headerFile->insertForwardDeclaration(" "), false);
                QCOMPARE(headerFile->insertForwardDeclaration("Foo::Bar::FooBar"), false);
                QCOMPARE(headerFile->insertForwardDeclaration("struct Neo"), true);
                QCOMPARE(headerFile->insertForwardDeclaration("struct SFoo::SBar::Uno"), true);
                headerFile->save();
            });
            QVERIFY(file.compare());
        }
    }

    void gotoBlockStartEnd()
    {
        Test::testCppDocument("tst_cppdocument/blockStartEnd", "source.cpp", [](auto *document) {
            // When cursor position is at the beginning of block
            document->setPosition(0);
            QCOMPARE(document->gotoBlockStart(), 0);
            QCOMPARE(document->gotoBlockEnd(), 0);

            // When cursor position is at the end of block
            document->setPosition(419);
            QCOMPARE(document->gotoBlockStart(), 53);
            QCOMPARE(document->gotoBlockEnd(), 495);

            // When cursor position is in between blocks
            document->setPosition(57);
            QCOMPARE(document->gotoBlockStart(), 53);
            QCOMPARE(document->gotoBlockEnd(), 495);

            document->setPosition(70);
            QCOMPARE(document->gotoBlockStart(), 64);
            QCOMPARE(document->gotoBlockEnd(), 115);

            document->setPosition(330);
            QCOMPARE(document->gotoBlockStart(), 311);
            QCOMPARE(document->gotoBlockEnd(), 390);
        });
    }

    void commentSelection_data()
    {
        QTest::addColumn<int>("regionStartPos");
        QTest::addColumn<int>("regionEndPos");
        QTest::addColumn<QString>("resultantFilePath");

        // #1.1 Selection starts and ends between characters - on different lines
        QTest::newRow("01-1-multi-line-between-to-between") << 23 << 53 << "main-01.cpp";

        // #1.2 Selection starts and ends between characters - on different lines - with range set in reverse order
        QTest::newRow("01-2-multi-line-between-to-between-reverse") << 53 << 23 << "main-01.cpp";

        // #2 Selection starts before characters and ends between characters - on different lines
        QTest::newRow("02-multi-line-before-to-between") << 15 << 53 << "main-02.cpp";

        // #3 Selection starts between characters and ends before characters - on different lines
        QTest::newRow("03-multi-line-between-to-before") << 23 << 45 << "main-03.cpp";

        // #4 Selection starts and ends before characters - on different lines
        QTest::newRow("04-multi-line-before-to-before") << 15 << 45 << "main-04.cpp";

        // #5 Selection starts and ends between characters - on same line
        QTest::newRow("05-single-line-between-to-between") << 18 << 23 << "main-05.cpp";

        // #6 Selection starts before characters and ends between characters - on same line
        QTest::newRow("06-single-line-before-to-between") << 15 << 23 << "main-06.cpp";

        // #7 Selection starts and ends before characters - on same line
        QTest::newRow("07-single-line-before-to-before") << 14 << 16 << "main-07.cpp";

        // #8 There is no selection - but the position is valid
        QTest::newRow("08-no-selection-valid-position") << 30 << -1 << "main-08.cpp";

        // #9 There is no selection - the position is valid - but on an empty line
        QTest::newRow("09-no-selection-valid-position-empty-line") << 58 << -1 << "main-no-change.cpp";
    }

    void commentSelection()
    {
        QFETCH(int, regionStartPos);
        QFETCH(int, regionEndPos);
        QFETCH(QString, resultantFilePath);

        resultantFilePath = Test::testDataPath() + "/tst_cppdocument/commentSelection/" + resultantFilePath;

        Test::testCppDocument("tst_cppdocument/commentSelection", "main.cpp",
                              [&regionStartPos, &regionEndPos, &resultantFilePath](auto *document) {
                                  if (regionEndPos == -1) {
                                      document->setPosition(regionStartPos);
                                  } else {
                                      document->selectRegion(regionStartPos, regionEndPos);
                                  }
                                  document->commentSelection();
                                  document->save();
                                  QVERIFY(Test::compareFiles(document->fileName(), resultantFilePath));
                                  document->undo();
                                  document->save();
                              });
    }

    void addMember()
    {
        Test::FileTester sourceFile(Test::testDataPath() + "/tst_cppdocument/addMember/addmember.cpp");
        Test::testCppDocument("tst_cppdocument/addMember", sourceFile.fileName(), [](auto *cppFile) {
            cppFile->addMember("QString foo", "Student", Core::CppDocument::AccessSpecifier::Public);
            cppFile->addMember("int bar", "Student", Core::CppDocument::AccessSpecifier::Protected);

            cppFile->save();
        });
        QVERIFY(sourceFile.compare());
    }

    void addMethodDeclaration()
    {
        Test::FileTester sourceFile(Test::testDataPath() + "/tst_cppdocument/addMethodDeclaration/addmethoddecl.cpp");
        Test::testCppDocument("/tst_cppdocument/addMethodDeclaration", sourceFile.fileName(), [](auto *cppFile) {
            cppFile->addMethodDeclaration("bool func(QString s, int a)", "Student",
                                          Core::CppDocument::AccessSpecifier::Public);
            cppFile->addMethodDeclaration("bool foo(QString s, int a)", "Student",
                                          Core::CppDocument::AccessSpecifier::Protected);

            cppFile->save();
        });
        QVERIFY(sourceFile.compare());
    }

    void addMethodDefinition()
    {

        Test::FileTester sourceFile(Test::testDataPath() + "/tst_cppdocument/addMethodDefinition/addmethoddef.cpp");
        Test::testCppDocument("/tst_cppdocument/addMethodDefinition", sourceFile.fileName(), [](auto *source) {
            QVERIFY(source->addMethodDefinition("bool func(std::string s, int a)", "Student"));
            // Test that addMethodDefinition can also strip declaration specifiers (i.e. override, etc.)
            QVERIFY(source->addMethodDefinition("static Q_SLOT virtual bool func(const QString &str) const override",
                                                "Student", "std::cout << str.toStdString() << std::endl;"));

            source->save();
        });
        QVERIFY(sourceFile.compare());
    }

    void queryMethod()
    {
        Test::testCppDocument("projects/cpp-project", "myobject.cpp", [](auto *document) {
            const auto methods = document->queryMethodDefinition("MyObject", "sayMessage");
            QCOMPARE(methods.size(), 2);

            QCOMPARE(methods[0].get("name").text(), "sayMessage");
            QCOMPARE(methods[0].get("return").text(), "void");
            QCOMPARE(methods[0].getAll("parameters").size(), 0);
            QCOMPARE(methods[0].get("parameter-list").text(), "()");
            QCOMPARE(methods[0].get("body").text(), "{\n    std::cout << m_message << std::endl;\n}");

            QCOMPARE(methods[1].get("name").text(), "sayMessage");
            QCOMPARE(methods[1].get("return").text(), "void");
            QCOMPARE(methods[1].getAll("parameters").size(), 1);
            QCOMPARE(methods[1].get("parameter-list").text(), "(const std::string& test)");
            QVERIFY(methods[1].get("body").text().contains("m_enum = MyEnum::C"));
        });
    }

    void queryFunctionCall()
    {
        Test::testCppDocument("projects/cpp-project", "main.cpp", [](auto *document) {
            auto calls = document->queryFunctionCall("object.sayMessage", {"message"});
            QCOMPARE(calls.size(), 1);
            // All nodes, including comments will be captured as the parameter
            const auto messageNodes = calls[0].getAll("message");
            QCOMPARE(messageNodes.size(), 2);

            QCOMPARE(calls[0].get("name").text(), "object.sayMessage");
            QCOMPARE(messageNodes[0].text(), "\"Another message\"");
            QCOMPARE(messageNodes[1].text(), "/*a comment*/");
            QCOMPARE(calls[0].get("argument-list").text(), "(\"Another message\" /*a comment*/)");

            calls = document->queryFunctionCall("object.sayMessage", {});
            QCOMPARE(calls.size(), 1);

            QCOMPARE(calls[0].get("name").text(), "object.sayMessage");
            QCOMPARE(calls[0].get("argument-list").text(), "()");

            calls = document->queryFunctionCall("object.sayMessage");
            QCOMPARE(calls.size(), 2);

            QCOMPARE(calls[0].get("name").text(), "object.sayMessage");
            QCOMPARE(calls[0].get("argument-list").text(), "()");
            QCOMPARE(calls[0].getAll("arguments").size(), 0);

            QCOMPARE(calls[1].get("name").text(), "object.sayMessage");
            QCOMPARE(calls[1].get("argument-list").text(), "(\"Another message\" /*a comment*/)");
            QCOMPARE(calls[1].getAll("arguments").size(), 1);
            QCOMPARE(calls[1].getAll("arguments")[0].text(), "\"Another message\"");
        });
    }

    // Regression test:
    // Putting the cursor before the last character made `moveBlock` run into an infinite loop.
    void selectBlockUpAtEndOfFile()
    {
        Test::testCppDocument("/tst_cppdocument/blockStartEnd", "source.cpp", [](auto *document) {
            auto textEdit = document->textEdit();

            auto cursor = textEdit->textCursor();
            auto lastCharacterPos = textEdit->document()->characterCount() - 1;

            // Placing the cursor right at the end was caught, but placing it one character before that wasn't.
            cursor.setPosition(lastCharacterPos - 1);
            textEdit->setTextCursor(cursor);

            document->selectBlockUp();
            QCOMPARE(document->selectBlockUp(), lastCharacterPos - 1);
        });
    }

    void mfcReplaceAfxMsgDeclaration()
    {
        Test::FileTester headerFile(Test::testDataPath() + "/tst_cppdocument/message_map/afx_msg_declaration.h");
        Test::testCppDocument("/tst_cppdocument/message_map", "afx_msg_declaration.h", [](auto *header) {
            auto funcDecl = header->queryMethodDeclaration("CTutorialDlg", "OnTimer");
            QCOMPARE(funcDecl.size(), 1);
            funcDecl.first().get("declaration").replace("void timerEvent(QTimerEvent *event) override;");
            header->save();
        });
        QVERIFY(headerFile.compare());
    }

    void keywords()
    {
        Test::testCppDocument("projects/cpp-project", "main.cpp", [](auto *document) {
            const QStringList keywords = document->keywords();
            QVERIFY(keywords.count() > 0);
            QVERIFY(keywords.contains("delete"));
            QCOMPARE(keywords, Core::Utils::cppKeywords());
        });
    }

    void primitiveTypes()
    {
        Test::testCppDocument("projects/cpp-project", "main.cpp", [](auto *document) {
            const QStringList primitiveTypes = document->primitiveTypes();
            QVERIFY(primitiveTypes.count() > 0);
            QVERIFY(primitiveTypes.contains("wchar_t"));
            QCOMPARE(primitiveTypes, Core::Utils::cppPrimitiveTypes());
        });
    }
};

QTEST_MAIN(TestCppDocument)
#include "tst_cppdocument.moc"
