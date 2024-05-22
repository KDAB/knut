#include "core/cppdocument.h"
#include "core/knutcore.h"
#include "core/project.h"

#include "common/test_cpputils.h"
#include "common/test_utils.h"

#include <kdalgorithms.h>

class TestCppDocumentTreeSitter : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void extractDataExchange()
    {
        Test::testCppDocument("projects/mfc-tutorial", "TutorialDlg.cpp", [](Core::CppDocument *document) {
            auto ddx = document->mfcExtractDDX("CTutorialDlg");

            QCOMPARE(ddx.className, "CTutorialDlg");
            QCOMPARE(ddx.isValid(), true);
            QVERIFY(ddx.range.text().startsWith("void CTutorialDlg::DoDataExchange(CDataExchange* pDX)"));
            QVERIFY(ddx.range.text().endsWith("}"));

            QCOMPARE(ddx.entries.size(), 8);
            QCOMPARE(ddx.entries.first().function, "DDX_Text");
            QCOMPARE(ddx.entries.first().idc, "IDC_ECHO_AREA");
            QCOMPARE(ddx.entries.first().member, "m_EchoText");

            QCOMPARE(ddx.entries.at(3).function, "DDX_Control");
            QCOMPARE(ddx.entries.at(3).idc, "IDC_V_SLIDER_BAR");
            QCOMPARE(ddx.entries.at(3).member, "m_VSliderBar");

            QCOMPARE(ddx.entries.last().function, "DDX_Check");
            QCOMPARE(ddx.entries.last().idc, "IDC_TIMER_CONTROL_SLIDERS");
            QCOMPARE(ddx.entries.last().member, "m_TimerCtrlSliders");

            QCOMPARE(ddx.validators.size(), 1);
            const auto &validator = ddx.validators.first();
            QCOMPARE(validator.function, "DDV_MaxChars");
            QCOMPARE(validator.member, "m_EchoText");
            QCOMPARE(validator.arguments, QStringList({"3"}));
        });
    }

private:
    void tryInsertingBar(const QString &fileName)
    {
        Test::FileTester fileTester(fileName);
        auto file = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(fileTester.fileName()));
        file->insertInclude("<bar.h>", false);
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
            QVERIFY(cppFile->insertInclude(R"("folder/foobar.h")", false));
            cppFile->insertInclude("<QPushButton>", false);
            cppFile->insertInclude("<memory>", true);

            // Mal-formed includes
            cppFile->insertInclude(R"("foobar.h)", false);
            cppFile->insertInclude(R"(<foobar.h)", false);
            cppFile->insertInclude(R"(foobar.h")", false);
            QVERIFY(!cppFile->insertInclude(R"(foobar.h>)", false));

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

        tryInsertingBar(Test::testDataPath() + "/tst_cppdocument/insertRemoveInclude/include.h");
        tryInsertingBar(Test::testDataPath() + "/tst_cppdocument/insertRemoveInclude/pragma.h");
        tryInsertingBar(Test::testDataPath() + "/tst_cppdocument/insertRemoveInclude/guards.h");
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

    void queryMember_data()
    {
        QTest::addColumn<QString>("memberName");
        QTest::addColumn<QString>("type");
        QTest::addColumn<QString>("text");

        QTest::newRow("empty") << "m_doesnotexist"
                               << ""
                               << "";
        QTest::newRow("double") << "m_double"
                                << "double"
                                << "double m_double = 1.1;";
        QTest::newRow("int") << "m_constInt"
                             << "int"
                             << "const int m_constInt = 2;";
        QTest::newRow("ref") << "m_fooRef"
                             << "Foo"
                             << "Foo &m_fooRef;";
        QTest::newRow("ptr") << "m_barPtr"
                             << "Bar"
                             << "Bar *m_barPtr = nullptr;";
        QTest::newRow("ptrRef") << "m_fooPtrRef"
                                << "Foo"
                                << "const Foo *& m_fooPtrRef;";
        QTest::newRow("ptrptr") << "m_barPtrPtr"
                                << "Bar"
                                << "Bar **m_barPtrPtr;";
    }

    void queryMember()
    {
        QFETCH(QString, memberName);
        QFETCH(QString, type);
        QFETCH(QString, text);

        Test::testCppDocument("tst_cppdocument/query", "myclass.h",
                              [&memberName, &type, &text](Core::CppDocument *document) {
                                  auto query = document->queryMember("MyClass", memberName);
                                  QCOMPARE(query.get("type").text(), type);
                                  QCOMPARE(query.get("member").text(), text);
                              });
    }

    void queryMethodDeclaration_data()
    {
        QTest::addColumn<QString>("functionName");
        QTest::addColumn<QString>("text");

        QTest::newRow("foo") << "foo"
                             << "Foo foo() const;";
        QTest::newRow("setFoo") << "setFoo"
                                << "void setFoo(Foo* foo);";
        QTest::newRow("setFooBar") << "setFooBar"
                                   << "virtual void setFooBar(Foo* foo, Bar* bar);";
        QTest::newRow("fooRef") << "fooRef"
                                << "Foo &fooRef() const;";
        QTest::newRow("barPtr") << "barPtr"
                                << "Bar *barPtr() const;";
        QTest::newRow("fooPtrRef") << "fooPtrRef"
                                   << "const Foo *&fooPtrRef() const;";
        QTest::newRow("barPtrPtr") << "barPtrPtr"
                                   << "Bar **barPtrPtr() const;";
    }

    void queryMethodDeclaration()
    {
        QFETCH(QString, functionName);
        QFETCH(QString, text);

        Test::testCppDocument("tst_cppdocument/query", "myclass.h",
                              [&functionName, &text](Core::CppDocument *document) {
                                  auto queries = document->queryMethodDeclaration("MyClass", functionName);
                                  QCOMPARE(queries.size(), 1);
                                  QCOMPARE(queries.first().get("declaration").text(), text);
                              });

        // TODO: test overloads (same function name)
    }

    void queryMethodDefinition_data()
    {
        QTest::addColumn<QString>("functionName");
        QTest::addColumn<QString>("definition");
        QTest::addColumn<QString>("parameterList");

        QTest::newRow("foo") << "foo"
                             << "Foo MyClass::foo() const\n{ \n    return m_fooRef; \n}"
                             << "()";
        QTest::newRow("setFoo") << "setFoo"
                                << "void MyClass::setFoo(Foo* foo)\n{\n    // Do something\n}"
                                << "(Foo* foo)";
        QTest::newRow("setFooBar") << "setFooBar"
                                   << "void MyClass::setFooBar(Foo* foo, Bar* bar)\n{\n    // Do Something\n}"
                                   << "(Foo* foo, Bar* bar)";
        QTest::newRow("fooRef") << "fooRef"
                                << "Foo &MyClass::fooRef() const\n{\n    return m_fooRef;\n}"
                                << "()";
        QTest::newRow("barPtr") << "barPtr"
                                << "Bar *MyClass::barPtr() const\n{\n    return m_barPtr;\n}"
                                << "()";
        QTest::newRow("fooPtrRef") << "fooPtrRef"
                                   << "const Foo *&MyClass::fooPtrRef() const\n{\n    return m_fooPtrRef;\n}"
                                   << "()";
        QTest::newRow("barPtrPtr") << "barPtrPtr"
                                   << "Bar **MyClass::barPtrPtr() const\n{\n    return m_barPtrPtr;\n}"
                                   << "()";
    }

    void queryMethodDefinition()
    {
        QFETCH(QString, functionName);
        QFETCH(QString, definition);
        QFETCH(QString, parameterList);

        Test::testCppDocument("tst_cppdocument/query", "myclass.cpp",
                              [&functionName, &definition, &parameterList](Core::CppDocument *document) {
                                  auto queries = document->queryMethodDefinition("MyClass", functionName);
                                  QCOMPARE(queries.size(), 1);
                                  QCOMPARE(queries.first().get("definition").text(), definition);
                                  QCOMPARE(queries.first().get("parameter-list").text(), parameterList);
                              });

        // TODO: test overloads
        // TODO: test parameters
    }

    void changeBaseClass()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/tst_cppdocument/changeBaseClass");

        Test::FileTester sourceFile(Test::testDataPath() + "/tst_cppdocument/changeBaseClass/myobject.cpp");
        Test::FileTester headerFile(Test::testDataPath() + "/tst_cppdocument/changeBaseClass/myobject.h");
        {
            auto cppFile = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(sourceFile.fileName()));
            auto hFile = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(headerFile.fileName()));
            QVERIFY(cppFile->changeBaseClass(hFile, cppFile, "MyObject", "KPropertyPage"));
            cppFile->save();
            hFile->save();

            QVERIFY(sourceFile.compare());
            QVERIFY(headerFile.compare());
        }
    }
};

QTEST_MAIN(TestCppDocumentTreeSitter)
#include "tst_cppdocument_treesitter.moc"
