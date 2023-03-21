#include "core/cppdocument.h"
#include "core/knutcore.h"
#include "core/project.h"

#include "common/test_utils.h"

#include <kdalgorithms.h>

class TestCppDocumentTreeSitter : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

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

    void addMember()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/tst_cppdocument/addMember");

        Test::FileTester sourceFile(Test::testDataPath() + "/tst_cppdocument/addMember/addmember.cpp");
        {
            auto cppFile = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(sourceFile.fileName()));
            cppFile->addMember("QString foo", "Student", Core::CppDocument::AccessSpecifier::Public);
            cppFile->addMember("int bar", "Student", Core::CppDocument::AccessSpecifier::Protected);

            cppFile->save();
            QVERIFY(sourceFile.compare());
        }
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

QTEST_MAIN(TestCppDocumentTreeSitter)
#include "tst_cppdocument_treesitter.moc"
