#include "core/cppdocument.h"
#include "core/knutcore.h"
#include "core/project.h"

#include "common/test_utils.h"

#include <QPlainTextEdit>
#include <QTest>
#include <QThread>

#include <kdalgorithms.h>

class TestCppDocument : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void correspondingHeaderSource_data()
    {
        QTest::addColumn<QString>("headerOrSource");
        QTest::addColumn<QString>("sourceOrHeader");

        QTest::newRow("cpp") << Test::testDataPath() + "/tst_cppdocument/headerSource/test/hello.cpp"
                             << Test::testDataPath() + "/tst_cppdocument/headerSource/test/hello.h";
        QTest::newRow("h") << Test::testDataPath() + "/tst_cppdocument/headerSource/test/hello.h"
                           << Test::testDataPath() + "/tst_cppdocument/headerSource/test/hello.cpp";
        QTest::newRow("cxx") << Test::testDataPath() + "/tst_cppdocument/headerSource/test/world.cxx"
                             << Test::testDataPath() + "/tst_cppdocument/headerSource/test/world.hpp";
        QTest::newRow("folder") << Test::testDataPath() + "/tst_cppdocument/headerSource/folder1/foo.cpp"
                                << Test::testDataPath() + "/tst_cppdocument/headerSource/folder2/foo.h";
        QTest::newRow("subfolder") << Test::testDataPath() + "/tst_cppdocument/headerSource/test/subfolder1/foo.cpp"
                                   << Test::testDataPath() + "/tst_cppdocument/headerSource/test/subfolder2/foo.h";
        QTest::newRow("notexist") << Test::testDataPath() + "/tst_cppdocument/headerSource/test/bar.cpp"
                                  << "";
    }

    void correspondingHeaderSource()
    {
        QFETCH(QString, headerOrSource);
        QFETCH(QString, sourceOrHeader);

        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/tst_cppdocument/headerSource");
        auto document = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(headerOrSource));
        QCOMPARE(sourceOrHeader, document->correspondingHeaderSource());
    }

    void insertForwardDeclaration()
    {
        Test::FileTester file(Test::testDataPath() + "/tst_cppdocument/forwardDeclaration/header.h");
        {
            Core::KnutCore core;
            Core::Project::instance()->setRoot(Test::testDataPath() + "/tst_cppdocument/forwardDeclaration");

            auto headerFile = qobject_cast<Core::CppDocument *>(Core::Project::instance()->get(file.fileName()));
            QCOMPARE(headerFile->insertForwardDeclaration("class Foo"), true);
            QCOMPARE(headerFile->insertForwardDeclaration("class Foo::Bar::FooBar"), true);
            QCOMPARE(headerFile->insertForwardDeclaration("class Foo::Bar::FooBar"), false);
            QCOMPARE(headerFile->insertForwardDeclaration(" "), false);
            QCOMPARE(headerFile->insertForwardDeclaration("Foo::Bar::FooBar"), false);
            QCOMPARE(headerFile->insertForwardDeclaration("struct Neo"), true);
            QCOMPARE(headerFile->insertForwardDeclaration("struct SFoo::SBar::Uno"), true);
            headerFile->save();
            QVERIFY(file.compare());
        }
    }

    void extractDataExchange()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/projects/mfc-tutorial");

        auto document = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open("TutorialDlg.cpp"));
        auto ddxMap = document->mfcExtractDDX("CTutorialDlg");

        QCOMPARE(ddxMap.size(), 8);
        QCOMPARE(ddxMap.value("IDC_ECHO_AREA"), "m_EchoText");
        QCOMPARE(ddxMap.value("IDC_MOUSEECHO"), "m_MouseEcho");
    }

    void gotoBlockStartEnd()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/tst_cppdocument/blockStartEnd");

        auto document = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open("source.cpp"));

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
    }

    void commentSelection_data()
    {
        QTest::addColumn<int>("regionStartPos");
        QTest::addColumn<int>("regionEndPos");
        QTest::addColumn<QString>("resultantFilePath");

        // #1.1 Selection starts and ends between characters - on different lines
        QTest::newRow("01-1-multi-line-between-to-between")
            << 23 << 53 << (Test::testDataPath() + "/tst_cppdocument/commentSelection/main-01.cpp");

        // #1.2 Selection starts and ends between characters - on different lines - with range set in reverse order
        QTest::newRow("01-2-multi-line-between-to-between-reverse")
            << 53 << 23 << (Test::testDataPath() + "/tst_cppdocument/commentSelection/main-01.cpp");

        // #2 Selection starts before characters and ends between characters - on different lines
        QTest::newRow("02-multi-line-before-to-between")
            << 15 << 53 << (Test::testDataPath() + "/tst_cppdocument/commentSelection/main-02.cpp");

        // #3 Selection starts between characters and ends before characters - on different lines
        QTest::newRow("03-multi-line-between-to-before")
            << 23 << 45 << (Test::testDataPath() + "/tst_cppdocument/commentSelection/main-03.cpp");

        // #4 Selection starts and ends before characters - on different lines
        QTest::newRow("04-multi-line-before-to-before")
            << 15 << 45 << (Test::testDataPath() + "/tst_cppdocument/commentSelection/main-04.cpp");

        // #5 Selection starts and ends between characters - on same line
        QTest::newRow("05-single-line-between-to-between")
            << 18 << 23 << (Test::testDataPath() + "/tst_cppdocument/commentSelection/main-05.cpp");

        // #6 Selection starts before characters and ends between characters - on same line
        QTest::newRow("06-single-line-before-to-between")
            << 15 << 23 << (Test::testDataPath() + "/tst_cppdocument/commentSelection/main-06.cpp");

        // #7 Selection starts and ends before characters - on same line
        QTest::newRow("07-single-line-before-to-before")
            << 14 << 16 << (Test::testDataPath() + "/tst_cppdocument/commentSelection/main-07.cpp");

        // #8 There is no selection - but the position is valid
        QTest::newRow("08-no-selection-valid-position")
            << 30 << -1 << (Test::testDataPath() + "/tst_cppdocument/commentSelection/main-08.cpp");

        // #9 There is no selection - the position is valid - but on an empty line
        QTest::newRow("09-no-selection-valid-position-empty-line")
            << 58 << -1 << (Test::testDataPath() + "/tst_cppdocument/commentSelection/main-no-change.cpp");
    }

    void commentSelection()
    {
        QFETCH(int, regionStartPos);
        QFETCH(int, regionEndPos);
        QFETCH(QString, resultantFilePath);

        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/tst_cppdocument/commentSelection");

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

private:
    void tryInstertingBar(const QString &fileName)
    {
        Test::FileTester fileTester(fileName);
        auto file = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(fileTester.fileName()));
        file->insertInclude("<bar.h>");
        file->save();
        QVERIFY(fileTester.compare());
    }

private slots:
    void insertRemoveInclude()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/tst_cppdocument/insertRemoveInclude");

        Test::FileTester sourceFile(Test::testDataPath() + "/tst_cppdocument/insertRemoveInclude/include.cpp");
        {
            auto cppFile = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(sourceFile.fileName()));

            // Add include files
            QVERIFY(cppFile->insertInclude(R"("folder/foobar.h")"));
            cppFile->insertInclude("<QPushButton>");
            cppFile->insertInclude("<memory>", true);

            // Mal-formed includes
            cppFile->insertInclude(R"("foobar.h)");
            cppFile->insertInclude(R"(<foobar.h)");
            cppFile->insertInclude(R"(foobar.h")");
            QVERIFY(!cppFile->insertInclude(R"(foobar.h>)"));

            // Remove include files
            QVERIFY(cppFile->removeInclude(R"("bar.h")"));
            cppFile->removeInclude("<spdlog/spdlog.h>");
            cppFile->removeInclude("<QComboBox>");

            // Mal-formed includes
            cppFile->removeInclude(R"("foobar.h)");
            cppFile->removeInclude(R"(<foobar.h)");
            cppFile->removeInclude(R"(foobar.h")");
            QVERIFY(!cppFile->removeInclude(R"(foobar.h>)"));

            cppFile->save();
            QVERIFY(sourceFile.compare());
        }

        tryInstertingBar(Test::testDataPath() + "/tst_cppdocument/insertRemoveInclude/include.h");
        tryInstertingBar(Test::testDataPath() + "/tst_cppdocument/insertRemoveInclude/pragma.h");
        tryInstertingBar(Test::testDataPath() + "/tst_cppdocument/insertRemoveInclude/guards.h");
    }

    // Regression test:
    // Putting the cursor before the last character made `moveBlock` run into an infinite loop.
    void selectBlockUpAtEndOfFile()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/tst_cppdocument/blockStartEnd");

        auto document = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open("source.cpp"));
        auto textEdit = document->textEdit();

        auto cursor = textEdit->textCursor();
        auto lastCharacterPos = textEdit->document()->characterCount() - 1;

        // Placing the cursor right at the end was caught, but placing it one character before that wasn't.
        cursor.setPosition(lastCharacterPos - 1);
        textEdit->setTextCursor(cursor);

        document->selectBlockUp();
        QCOMPARE(document->selectBlockUp(), lastCharacterPos - 1);
    }

private:
    void messageMapForNonExistingClass(Core::CppDocument *cppdocument)
    {

        auto messageMap = cppdocument->mfcExtractMessageMap("NonExistentClass");
        QVERIFY(!messageMap.isValid());
        QVERIFY(messageMap.entries.isEmpty());
        QVERIFY(messageMap.className.isEmpty());
        QVERIFY(!messageMap.range.isValid());
    }

    void existingMessageMap(Core::CppDocument *cppdocument)
    {
        auto messageMap = cppdocument->mfcExtractMessageMap("CTutorialDlg");

        const QVector<std::pair<QString, QStringList>> expectedEntries = {
            {"ON_WM_PAINT", {}},
            {"ON_WM_HSCROLL", {}},
            {"ON_WM_VSCROLL", {}},
            {"ON_WM_TIMER", {}},
            {"ON_WM_LBUTTONDOWN", {}},
            {"ON_WM_MOUSEMOVE", {}},
            {"ON_WM_RBUTTONDOWN", {}},
            {"ON_BN_CLICKED", {"ID_BTN_ADD", "OnBnClickedBtnAdd"}},
            {"ON_BN_CLICKED", {"IDC_TIMER_CONTROL_SLIDERS", "OnBnClickedTimerControlSliders"}}};

        QVERIFY(messageMap.isValid());
        QCOMPARE(messageMap.className, QString("CTutorialDlg"));
        QCOMPARE(messageMap.superClass, QString("CDialog"));
        QCOMPARE(messageMap.entries.size(), expectedEntries.size());

        for (int i = 0; i < expectedEntries.size(); ++i) {
            const auto &expectedEntry = expectedEntries[i];
            const auto &actualEntry = messageMap.entries[i];

            auto toString = [](const Core::RangeMark &range) {
                return range.text();
            };
            QCOMPARE(actualEntry.name, expectedEntry.first);
            QCOMPARE(kdalgorithms::transformed(actualEntry.parameters, toString), expectedEntry.second);
        }
    }

private slots:
    void mfcExtractMessageMap()
    {
        Core::KnutCore core;
        auto project = Core::Project::instance();
        project->setRoot(Test::testDataPath() + "/projects/mfc-tutorial");

        auto cppdocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->get("TutorialDlg.cpp"));

        messageMapForNonExistingClass(cppdocument);

        existingMessageMap(cppdocument);
    }

    void deleteMessageMap()
    {
        Test::FileTester file(Test::testDataPath() + "/tst_cppdocument/message_map/TutorialDlg.cpp");
        {
            Core::KnutCore core;
            auto project = Core::Project::instance();
            project->setRoot(Test::testDataPath() + "/cpp-project");

            auto cppdocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->get(file.fileName()));
            QVERIFY(cppdocument);

            auto messageMap = cppdocument->mfcExtractMessageMap("CTutorialDlg");

            QVERIFY(messageMap.isValid());
            messageMap.range.select();
            cppdocument->deleteSelection();

            cppdocument->save();
            QVERIFY(file.compare());

            // TODO: This line crash the test on the CI, comment out for now
            // QVERIFY(!cppdocument->mfcExtractMessageMap("CTutorialDlg").isValid());
        }
    }
};

QTEST_MAIN(TestCppDocument)
#include "tst_cppdocument.moc"
