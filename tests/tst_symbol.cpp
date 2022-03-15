#include "core/cppclass.h"
#include "core/cppdocument.h"
#include "core/knutcore.h"
#include "core/project.h"
#include "core/symbol.h"

#include "common/test_utils.h"

#include <QTest>
#include <QThread>

class TestSymbol : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void toFunction_data()
    {
        QTest::addColumn<Core::Symbol>("symbol");
        QTest::addColumn<Core::CppFunction>("cppFunction");

        // test #1
        auto symbol1 = Core::Symbol {.name = "isHeaderSuffix",
                                     .description = "static bool (const QString &)",
                                     .kind = Core::Symbol::Kind::Method,
                                     .range = Core::TextRange {.start = 0, .end = 1},
                                     .selectionRange = Core::TextRange {.start = 10, .end = 11}};
        auto cppFunction1 =
            Core::CppFunction {.name = "isHeaderSuffix",
                               .returnType = "bool",
                               .arguments = QVector<Core::Argument> {{.type = "const QString &", .name = ""}},
                               .range = Core::TextRange {.start = 0, .end = 1}};
        QTest::newRow("test-case-method-one-arg") << symbol1 << cppFunction1;

        // test #2
        auto symbol2 = Core::Symbol {.name = "candidateFileNames",
                                     .description = "static QStringList (const QString &, const QStringList &)",
                                     .kind = Core::Symbol::Kind::Method,
                                     .range = Core::TextRange {.start = 2, .end = 3},
                                     .selectionRange = Core::TextRange {.start = 12, .end = 13}};
        auto cppFunction2 =
            Core::CppFunction {.name = "candidateFileNames",
                               .returnType = "QStringList",
                               .arguments = QVector<Core::Argument> {{.type = "const QString &", .name = ""},
                                                                     {.type = "const QStringList &", .name = ""}},
                               .range = Core::TextRange {.start = 2, .end = 3}};
        QTest::newRow("test-case-method-two-args") << symbol2 << cppFunction2;

        // test #3
        auto symbol3 = Core::Symbol {.name = "CppDocument::correspondingHeaderSource",
                                     .description = "QString (int, bool) const",
                                     .kind = Core::Symbol::Kind::Function,
                                     .range = Core::TextRange {.start = 4, .end = 5},
                                     .selectionRange = Core::TextRange {.start = 14, .end = 15}};
        auto cppFunction3 = Core::CppFunction {
            .name = "CppDocument::correspondingHeaderSource",
            .returnType = "QString",
            .arguments = QVector<Core::Argument> {{.type = "int", .name = ""}, {.type = "bool", .name = ""}},
            .range = Core::TextRange {.start = 4, .end = 5}};
        QTest::newRow("test-case-function-two-args-const") << symbol3 << cppFunction3;

        // test #4
        auto symbol4 = Core::Symbol {.name = "Foo::Run",
                                     .description = "static const int &(World &, PrioQ *) volatile",
                                     .kind = Core::Symbol::Kind::Function,
                                     .range = Core::TextRange {.start = 6, .end = 7},
                                     .selectionRange = Core::TextRange {.start = 16, .end = 17}};
        auto cppFunction4 = Core::CppFunction {
            .name = "Foo::Run",
            .returnType = "const int &",
            .arguments = QVector<Core::Argument> {{.type = "World &", .name = ""}, {.type = "PrioQ *", .name = ""}},
            .range = Core::TextRange {.start = 6, .end = 7}};
        QTest::newRow("test-case-function-two-args-volatile") << symbol3 << cppFunction3;

        // test #5
        auto symbol5 = Core::Symbol {.name = "CppDocument::openHeaderSource",
                                     .description = "CppDocument *()",
                                     .kind = Core::Symbol::Kind::Method,
                                     .range = Core::TextRange {.start = 8, .end = 9},
                                     .selectionRange = Core::TextRange {.start = 18, .end = 19}};
        auto cppFunction5 = Core::CppFunction {.name = "CppDocument::openHeaderSource",
                                               .returnType = "CppDocument *",
                                               .arguments = QVector<Core::Argument>(),
                                               .range = Core::TextRange {.start = 8, .end = 9}};
        QTest::newRow("test-case-method-no-args") << symbol5 << cppFunction5;

        // test #6
        auto symbol6 = Core::Symbol {.name = "EnumMember",
                                     .description = "EnumMember = 0x01,",
                                     .kind = Core::Symbol::Kind::EnumMember,
                                     .range = Core::TextRange {.start = 1, .end = 2},
                                     .selectionRange = Core::TextRange {.start = 11, .end = 12}};
        auto cppFunction6 = Core::CppFunction();
        QTest::newRow("test-case-non-method-or-function") << symbol6 << cppFunction6;

        // test #7
        auto symbol7 = Core::Symbol {.name = "Foo::bar",
                                     .description = "void (const QHash<QString, QString> &)",
                                     .kind = Core::Symbol::Kind::Method,
                                     .range = Core::TextRange {.start = 3, .end = 4},
                                     .selectionRange = Core::TextRange {.start = 13, .end = 14}};
        auto cppFunction7 = Core::CppFunction {
            .name = "Foo::bar",
            .returnType = "void",
            .arguments = QVector<Core::Argument> {{.type = "const QHash<QString, QString> &", .name = ""}},
            .range = Core::TextRange {.start = 3, .end = 4}};
        QTest::newRow("test-case-method-one-arg-qhash-fail") << symbol7 << cppFunction7;
    }

    void toFunction()
    {
        QFETCH(Core::Symbol, symbol);
        QFETCH(Core::CppFunction, cppFunction);

        QEXPECT_FAIL("test-case-method-one-arg-qhash-fail", "This case is planned to be implemented later.", Continue);
        QCOMPARE(symbol.toFunction(), cppFunction);
    }

    void toClass()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/symbol/");

        QVector<Core::Symbol> members;
        members.append(Core::Symbol {.name = "Base::TestDocument::isParent",
                                     .description = "bool () const",
                                     .kind = Core::Symbol::Method,
                                     .range = Core::TextRange {.start = 86, .end = 107},
                                     .selectionRange = Core::TextRange {.start = 91, .end = 99}});
        members.append(Core::Symbol {.name = "Base::TestDocument::childrenCount",
                                     .description = "int (const int &)",
                                     .kind = Core::Symbol::Method,
                                     .range = Core::TextRange {.start = 113, .end = 147},
                                     .selectionRange = Core::TextRange {.start = 117, .end = 130}});
        members.append(Core::Symbol {.name = "Base::TestDocument::rows",
                                     .description = "int",
                                     .kind = Core::Symbol::Field,
                                     .range = Core::TextRange {.start = 154, .end = 162},
                                     .selectionRange = Core::TextRange {.start = 158, .end = 162}});
        members.append(Core::Symbol {.name = "Base::TestDocument::columns",
                                     .description = "int",
                                     .kind = Core::Symbol::Field,
                                     .range = Core::TextRange {.start = 168, .end = 179},
                                     .selectionRange = Core::TextRange {.start = 172, .end = 179}});
        auto testCppClass = Core::CppClass {.name = "Base::TestDocument", .members = std::move(members)};

        Core::Symbol testSymbol;
        auto cppDocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open("test_cppclass.h"));
        for (auto &symbol : cppDocument->symbols()) {
            if (symbol.kind == Core::Symbol::Class) {
                testSymbol = symbol;
                break;
            }
        }

        QCOMPARE(testSymbol.toClass(), testCppClass);
    }
};

QTEST_MAIN(TestSymbol)
#include "tst_symbol.moc"
