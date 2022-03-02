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
private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void followSymbol()
    {
        CHECK_CLANGD_VERSION;

        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/cpp-project/");
        auto lspdocument = qobject_cast<Core::LspDocument *>(Core::Project::instance()->open("main.cpp"));

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
        QCOMPARE(cursor.blockNumber(), 9); // lines are 0-indexed, so 9 => line 10
        QCOMPARE(cursor.selectedText(), QString("sayMessage"));

        // Selected a function definition -> goTo function declaration
        result = dynamic_cast<Core::LspDocument *>(result->followSymbol());
        QVERIFY(result);
        QVERIFY(result->fileName().endsWith("myobject.h"));
        cursor = result->textEdit()->textCursor();
        QCOMPARE(cursor.blockNumber(), 8); // lines are 0-indexed, so 9 => line 10
        QCOMPARE(cursor.selectedText(), QString("sayMessage"));
    }
};

QTEST_MAIN(TestLspDocument)
#include "tst_lspdocument.moc"
