#include "core/knutcore.h"
#include "core/lspdocument.h"
#include "core/project.h"

#include <QPlainTextEdit>
#include <QTest>
#include <qtestcase.h>

#include "common/test_utils.h"

class TestJsonTransforms : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void LspDocument_transform()
    {
        CHECK_CLANGD_VERSION;

        Test::FileTester tester(Test::testDataPath() + "/tst_jsontransforms/regexPattern/dot-to-arrow.cpp");
        {
            Core::KnutCore core;
            auto project = Core::Project::instance();
            project->setRoot(Test::testDataPath() + "/tst_jsontransforms/regexPattern");

            auto cppDocument = qobject_cast<Core::LspDocument *>(project->open(tester.fileName()));

            std::unordered_map<QString, QString> context {{"name", "object"}};
            cppDocument->transform(Test::testDataPath() + "/tst_jsontransforms/regexPattern/dot-to-arrow.json",
                                   context);
            cppDocument->save();

            QVERIFY(tester.compare());
        }
    }

    void LspDocument_transformSymbol()
    {
        Test::FileTester tester(Test::testDataPath() + "/tst_jsontransforms/transformSymbol/main.cpp");

        {
            Core::KnutCore core;
            auto project = Core::Project::instance();
            project->setRoot(Test::testDataPath() + "/tst_jsontransforms/transformSymbol");

            auto cppDocument = qobject_cast<Core::LspDocument *>(project->open(tester.fileName()));
            auto cursor = cppDocument->textEdit()->textCursor();
            cursor.setPosition(cppDocument->toPos(Lsp::Position {.line = 7, .character = 17}));
            cppDocument->textEdit()->setTextCursor(cursor);

            auto symbol = cppDocument->symbolUnderCursor();
            cppDocument->transformSymbol(
                symbol, Test::testDataPath() + "/tst_jsontransforms/transformSymbol/dot-to-arrow.json");

            cppDocument->save();

            QVERIFY(tester.compare());
        }
    }
};

QTEST_MAIN(TestJsonTransforms)
#include "tst_jsontransforms.moc"
