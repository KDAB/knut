/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"
#include "core/cppdocument.h"
#include "core/knutcore.h"
#include "core/project.h"

#include <QTest>

class TestCppDocumentClangd : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void insertCodeInMethod()
    {
        CHECK_CLANGD_VERSION;

        Test::FileTester file(Test::testDataPath() + "/tst_cppdocument/insertCodeInMethod/myobject.cpp");
        {
            Core::KnutCore core;
            Core::Project::instance()->setRoot(Test::testDataPath() + "/tst_cppdocument/insertCodeInMethod");

            auto header = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open("myobject.h"));
            // Insert code at the start of the method
            bool isCodeInserted =
                header->insertCodeInMethod("MyObject::sayMessage", "// added this new line at the start of this method",
                                           Core::CppDocument::StartOfMethod);
            QCOMPARE(isCodeInserted, false);

            auto source = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(file.fileName()));

            source->insertCodeInMethod("MyObject::sayMessage", "// added this new line at the start of this method",
                                       Core::CppDocument::StartOfMethod);
            source->insertCodeInMethod("sayMessage", "// added this new line at the end of this method",
                                       Core::CppDocument::EndOfMethod);

            source->insertCodeInMethod("displayString", "// added this new line at the start of this method",
                                       Core::CppDocument::StartOfMethod);
            source->insertCodeInMethod("displayString", "// added this new line at the end of this method",
                                       Core::CppDocument::EndOfMethod);
            source->save();

            QVERIFY(file.compare());
        }
    }

    void toggleSection()
    {
        CHECK_CLANGD_VERSION;

        Test::FileTester file(Test::testDataPath() + "/tst_cppdocument/toggleSection/section.cpp");
        {
            Core::KnutCore core;
            auto project = Core::Project::instance();
            project->setRoot(Test::testDataPath() + "/tst_cppdocument/toggleSection");

            auto cppFile = qobject_cast<Core::CppDocument *>(Core::Project::instance()->get(file.fileName()));

            // Comment out full methods (start from the end)
            cppFile->gotoLine(50);
            cppFile->toggleSection();
            cppFile->gotoLine(42);
            cppFile->toggleSection();
            cppFile->gotoLine(35);
            cppFile->toggleSection();
            cppFile->gotoLine(28);
            cppFile->toggleSection();
            cppFile->gotoLine(11);
            cppFile->toggleSection();
            cppFile->gotoLine(5);
            cppFile->toggleSection();

            // no-op
            cppFile->gotoLine(2);
            cppFile->toggleSection();

            // Uncomment function
            cppFile->gotoLine(28);
            cppFile->toggleSection();

            cppFile->save();
            QVERIFY(file.compare());
        }
    }

    void deleteMethod()
    {
        CHECK_CLANGD_VERSION;

        auto folder = Test::testDataPath() + "/tst_cppdocument/deleteMethod";
        Test::FileTester cppfile(folder + "/myobject.cpp");
        Test::FileTester headerfile(folder + "/myobject.h");

        {
            Core::KnutCore core;
            Core::Project::instance()->setRoot(folder);
            auto cpp = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(cppfile.fileName()));

            QVERIFY(cpp != nullptr);
            cpp->deleteMethod("MyObject::sayMessage", "void ()");

            cpp->save();
            auto header = cpp->openHeaderSource();
            QVERIFY(header);
            header->save();

            QVERIFY(cppfile.compare());
            QVERIFY(headerfile.compare());
        }
    }
};

QTEST_MAIN(TestCppDocumentClangd)
#include "tst_cppdocument_clangd.moc"
