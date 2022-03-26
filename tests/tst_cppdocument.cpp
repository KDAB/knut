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

        Test::FileTester file(Test::testDataPath() + "/cppdocument/forward_declaration_original.h");
        auto headerFile = qobject_cast<Core::CppDocument *>(Core::Project::instance()->get(file.fileName()));
        headerFile->load(file.fileName());
        QCOMPARE(headerFile->insertForwardDeclaration("class Foo"), true);
        QCOMPARE(headerFile->insertForwardDeclaration("class Foo::Bar::FooBar"), true);
        QCOMPARE(headerFile->insertForwardDeclaration("class Foo::Bar::FooBar"), false);
        QCOMPARE(headerFile->insertForwardDeclaration(" "), false);
        QCOMPARE(headerFile->insertForwardDeclaration("Foo::Bar::FooBar"), false);
        QCOMPARE(headerFile->insertForwardDeclaration("struct Neo"), true);
        QCOMPARE(headerFile->insertForwardDeclaration("struct SFoo::SBar::Uno"), true);
        headerFile->save();
        QVERIFY(file.compare());
        headerFile->close();
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

    void tstStartEndBlock()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/cppdocument");

        auto document = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open("testblock.cpp"));

        // When cursor position is at the beginning of block
        document->setPosition(0);
        QCOMPARE(document->gotoBlockStart(), 0);
        QCOMPARE(document->gotoBlockEnd(), 0);

        // When cursor position is at the end of block
        document->setPosition(419);
        QCOMPARE(document->gotoBlockStart(), 53);
        QCOMPARE(document->gotoBlockEnd(), 419);

        // When cursor position is in between blocks
        document->setPosition(57);
        QCOMPARE(document->gotoBlockStart(), 53);
        QCOMPARE(document->gotoBlockEnd(), 419);

        document->setPosition(70);
        QCOMPARE(document->gotoBlockStart(), 64);
        QCOMPARE(document->gotoBlockEnd(), 115);

        document->setPosition(330);
        QCOMPARE(document->gotoBlockStart(), 311);
        QCOMPARE(document->gotoBlockEnd(), 390);
    }

    void commentSelection_data()
    {
        QTest::addColumn<int>("regionStartPos");
        QTest::addColumn<int>("regionEndPos");
        QTest::addColumn<QString>("resultantFilePath");

        // #1.1 Selection starts and ends between characters - on different lines
        QTest::newRow("01-1-multi-line-between-to-between")
            << 23 << 53 << (Test::testDataPath() + "/cppdocument/comment_test/main-01.cpp");

        // #1.2 Selection starts and ends between characters - on different lines - with range set in reverse order
        QTest::newRow("01-2-multi-line-between-to-between-reverse")
            << 53 << 23 << (Test::testDataPath() + "/cppdocument/comment_test/main-01.cpp");

        // #2 Selection starts before characters and ends between characters - on different lines
        QTest::newRow("02-multi-line-before-to-between")
            << 15 << 53 << (Test::testDataPath() + "/cppdocument/comment_test/main-02.cpp");

        // #3 Selection starts between characters and ends before characters - on different lines
        QTest::newRow("03-multi-line-between-to-before")
            << 23 << 45 << (Test::testDataPath() + "/cppdocument/comment_test/main-03.cpp");

        // #4 Selection starts and ends before characters - on different lines
        QTest::newRow("04-multi-line-before-to-before")
            << 15 << 45 << (Test::testDataPath() + "/cppdocument/comment_test/main-04.cpp");

        // #5 Selection starts and ends between characters - on same line
        QTest::newRow("05-single-line-between-to-between")
            << 18 << 23 << (Test::testDataPath() + "/cppdocument/comment_test/main-05.cpp");

        // #6 Selection starts before characters and ends between characters - on same line
        QTest::newRow("06-single-line-before-to-between")
            << 15 << 23 << (Test::testDataPath() + "/cppdocument/comment_test/main-06.cpp");

        // #7 Selection starts and ends before characters - on same line
        QTest::newRow("07-single-line-before-to-before")
            << 14 << 16 << (Test::testDataPath() + "/cppdocument/comment_test/main-07.cpp");

        // #8 There is no selection - but the position is valid
        QTest::newRow("08-no-selection-valid-position")
            << 30 << -1 << (Test::testDataPath() + "/cppdocument/comment_test/main-08.cpp");

        // #9 There is no selection - the position is valid - but on an empty line
        QTest::newRow("09-no-selection-valid-position-empty-line")
            << 58 << -1 << (Test::testDataPath() + "/cppdocument/comment_test/main-no-change.cpp");
    }

    void commentSelection()
    {
        QFETCH(int, regionStartPos);
        QFETCH(int, regionEndPos);
        QFETCH(QString, resultantFilePath);

        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/cppdocument/comment_test");

        auto cppDocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open("main.cpp"));
        if (regionEndPos == -1) {
            cppDocument->setPosition(regionStartPos);
        } else {
            cppDocument->selectRegion(regionStartPos, regionEndPos);
        }
        cppDocument->commentSelection();
        cppDocument->save();
        QVERIFY(Test::compareFiles(cppDocument->fileName(), resultantFilePath));
        cppDocument->undo();
        cppDocument->save();
    }

    void insertCodeInMethod()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/cppdocument");

        if (auto cppDocument =
                qobject_cast<Core::CppDocument *>(Core::Project::instance()->open("insert_code_original.h"))) {

            // Insert code at the start of the method
            bool isCodeInserted =
                cppDocument->insertCodeInMethod("updateActions", "// added this new line at the start of this method",
                                                Core::CppDocument::StartOfMethod);
            QCOMPARE(isCodeInserted, false);
            cppDocument->close();
        }

        Test::FileTester file_cpp(Test::testDataPath() + "/cppdocument/insert_code_original.cpp");
        if (auto cppDocument =
                qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(file_cpp.fileName()))) {
            // NOTE: Code insertion won't work for this case because (as of now) LSP couldn't find a non-static function
            //       symbol in a C++ file.
            // Insert code at the end of the method
            if (cppDocument->insertCodeInMethod(
                    "MainWindow::nonStaticMethod",
                    "// added following new lines at the end of this method"
                    "\ncppDocument->commentSelection();"
                    "\ncppDocument->save();"
                    "\nQVERIFY(Test::compareFiles(cppDocument->fileName(), resultantFilePath));"
                    "\ncppDocument->undo(); cppDocument->save();",
                    Core::CppDocument::EndOfMethod))
                cppDocument->save();
            cppDocument->close();
        }

        if (auto cppDocument =
                qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(file_cpp.fileName()))) {
            // Insert code at the end of the method
            if (cppDocument->insertCodeInMethod("shortMethod", "// added this new line at the end of this method",
                                                Core::CppDocument::EndOfMethod))
                cppDocument->save();
            cppDocument->close();
        }

        if (auto cppDocument =
                qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(file_cpp.fileName()))) {
            // Insert code at the start of the method
            if (cppDocument->insertCodeInMethod("shortMethod", "// added this new line at the start of this method",
                                                Core::CppDocument::StartOfMethod))
                cppDocument->save();
            cppDocument->close();
        }

        if (auto cppDocument =
                qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(file_cpp.fileName()))) {
            // Insert code at the end of the method
            if (cppDocument->insertCodeInMethod(
                    "longMethod1",
                    "// added following new lines at the end of this method"
                    "\ncppDocument->commentSelection();"
                    "\ncppDocument->save();"
                    "\nQVERIFY(Test::compareFiles(cppDocument->fileName(), resultantFilePath));"
                    "\ncppDocument->undo(); cppDocument->save();",
                    Core::CppDocument::EndOfMethod))
                cppDocument->save();
            cppDocument->close();
        }

        if (auto cppDocument =
                qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(file_cpp.fileName()))) {
            // Insert code at the start of the method
            if (cppDocument->insertCodeInMethod(
                    "longMethod1",
                    "// added following new lines at the start of this method"
                    "\ncppDocument->commentSelection();"
                    "\ncppDocument->save();"
                    "\nQVERIFY(Test::compareFiles(cppDocument->fileName(), resultantFilePath));"
                    "\ncppDocument->undo(); cppDocument->save();",
                    Core::CppDocument::StartOfMethod))
                cppDocument->save();
            cppDocument->close();
        }

        if (auto cppDocument =
                qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(file_cpp.fileName()))) {
            // Insert code at the end of the method
            if (cppDocument->insertCodeInMethod(
                    "longMethod2",
                    "// added following new lines at the end of this method"
                    "\ncppDocument->commentSelection();"
                    "\ncppDocument->save();"
                    "\nQVERIFY(Test::compareFiles(cppDocument->fileName(), resultantFilePath));"
                    "\ncppDocument->undo(); cppDocument->save();"
                    "\n",
                    Core::CppDocument::EndOfMethod))
                cppDocument->save();
            cppDocument->close();
        }

        if (auto cppDocument =
                qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(file_cpp.fileName()))) {
            // Insert code at the start of the method
            if (cppDocument->insertCodeInMethod(
                    "longMethod2",
                    "// added following new lines at the start of this method"
                    "\ncppDocument->commentSelection();"
                    "\ncppDocument->save();"
                    "\nQVERIFY(Test::compareFiles(cppDocument->fileName(), resultantFilePath));"
                    "\ncppDocument->undo(); cppDocument->save();"
                    "\n",
                    Core::CppDocument::StartOfMethod))
                cppDocument->save();
            cppDocument->close();
        }

        QVERIFY(file_cpp.compare());
    }
};

QTEST_MAIN(TestCppDocument)
#include "tst_cppdocument.moc"
