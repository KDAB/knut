#include "core/classsymbol.h"
#include "core/cppfunctionsymbol.h"
#include "core/knutcore.h"
#include "core/lspdocument.h"
#include "core/project.h"
#include "core/symbol.h"

#include "common/test_utils.h"

#include <QTest>
#include <QThread>

namespace Core {

char *toString(const Argument &argument)
{
    return QTest::toString(QString("Type: '%1' Name: '%2'").arg(argument.type).arg(argument.name));
}

}

class TestSymbol : public QObject
{
    Q_OBJECT

    struct FunctionData
    {
        QString name;
        QString returnType;
        QVector<Core::Argument> arguments;
        Core::TextRange range;

        bool isNull() { return name.isEmpty(); }
    };

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void toFunctionWithoutLSP_data()
    {
        QTest::addColumn<Core::Symbol *>("symbol");
        QTest::addColumn<FunctionData>("functionData");

        // test #1
        auto symbol1 = Core::Symbol::makeSymbol(this, "isHeaderSuffix", "static bool (const QString &)",
                                                Core::Symbol::Kind::Method, Core::TextRange {.start = 0, .end = 1},
                                                Core::TextRange {.start = 10, .end = 11});

        auto functionData1 =
            FunctionData {.name = "isHeaderSuffix",
                          .returnType = "bool",
                          .arguments = QVector<Core::Argument> {{.type = "const QString &", .name = ""}},
                          .range = Core::TextRange {.start = 0, .end = 1}};
        QTest::newRow("test-case-method-one-arg") << symbol1 << functionData1;

        // test #2
        auto symbol2 = Core::Symbol::makeSymbol(this, "candidateFileNames",
                                                "static QStringList (const QString &, const QStringList &)",
                                                Core::Symbol::Kind::Method, Core::TextRange {.start = 2, .end = 3},
                                                Core::TextRange {.start = 12, .end = 13});
        auto functionData2 =
            FunctionData {.name = "candidateFileNames",
                          .returnType = "QStringList",
                          .arguments = QVector<Core::Argument> {{.type = "const QString &", .name = ""},
                                                                {.type = "const QStringList &", .name = ""}},
                          .range = Core::TextRange {.start = 2, .end = 3}};
        QTest::newRow("test-case-method-two-args") << symbol2 << functionData2;

        // test #3
        auto symbol3 = Core::Symbol::makeSymbol(
            this, "CppDocument::correspondingHeaderSource", "QString (int, bool) const", Core::Symbol::Kind::Function,
            Core::TextRange {.start = 4, .end = 5}, Core::TextRange {.start = 14, .end = 15});
        auto functionData3 = FunctionData {
            .name = "CppDocument::correspondingHeaderSource",
            .returnType = "QString",
            .arguments = QVector<Core::Argument> {{.type = "int", .name = ""}, {.type = "bool", .name = ""}},
            .range = Core::TextRange {.start = 4, .end = 5}};
        QTest::newRow("test-case-function-two-args-const") << symbol3 << functionData3;

        // test #4
        auto symbol4 = Core::Symbol::makeSymbol(this, "Foo::Run", "static const int &(World &, PrioQ *) volatile",
                                                Core::Symbol::Kind::Function, Core::TextRange {.start = 6, .end = 7},
                                                Core::TextRange {.start = 16, .end = 17});
        auto functionData4 = FunctionData {
            .name = "Foo::Run",
            .returnType = "const int &",
            .arguments = QVector<Core::Argument> {{.type = "World &", .name = ""}, {.type = "PrioQ *", .name = ""}},
            .range = Core::TextRange {.start = 6, .end = 7}};
        QTest::newRow("test-case-function-two-args-volatile") << symbol4 << functionData4;

        // test #5
        auto symbol5 = Core::Symbol::makeSymbol(this, "CppDocument::openHeaderSource", "CppDocument *()",
                                                Core::Symbol::Kind::Method, Core::TextRange {.start = 8, .end = 9},
                                                Core::TextRange {.start = 18, .end = 19});
        auto functionData5 = FunctionData {.name = "CppDocument::openHeaderSource",
                                           .returnType = "CppDocument *",
                                           .arguments = QVector<Core::Argument>(),
                                           .range = Core::TextRange {.start = 8, .end = 9}};
        QTest::newRow("test-case-method-no-args") << symbol5 << functionData5;

        // test #6
        auto symbol6 =
            Core::Symbol::makeSymbol(this, "EnumMember", "EnumMember = 0x01,", Core::Symbol::Kind::EnumMember,
                                     Core::TextRange {.start = 1, .end = 2}, Core::TextRange {.start = 11, .end = 12});
        auto functionData6 = FunctionData();
        QTest::newRow("test-case-non-method-or-function") << symbol6 << functionData6;

        // test #7
        auto symbol7 = Core::Symbol::makeSymbol(this, "Foo::bar", "void (const QHash<QString, QString> &)",
                                                Core::Symbol::Kind::Method, Core::TextRange {.start = 3, .end = 4},
                                                Core::TextRange {.start = 13, .end = 14});
        auto functionData7 = FunctionData {
            .name = "Foo::bar",
            .returnType = "void",
            .arguments = QVector<Core::Argument> {{.type = "const QHash<QString, QString> &", .name = ""}},
            .range = Core::TextRange {.start = 3, .end = 4}};
        QTest::newRow("test-case-method-one-arg-qhash-fail") << symbol7 << functionData7;
    }

    // This test case uses the fallback heuristic that doesn't use
    // any additional LSP calls.
    // This is forced by not setting the parent of the test cases
    // to an LSPDocument.
    void toFunctionWithoutLSP()
    {
        QFETCH(Core::Symbol *, symbol);
        QFETCH(FunctionData, functionData);

        auto cppFunction = symbol->toFunction();
        if (functionData.isNull()) {
            QVERIFY(!cppFunction);
            return;
        }

        QVERIFY(cppFunction);
        QCOMPARE(cppFunction->name(), functionData.name);
        QCOMPARE(cppFunction->returnType(), functionData.returnType);
        QEXPECT_FAIL("test-case-method-one-arg-qhash-fail", "This case is planned to be implemented later.", Continue);
        QCOMPARE(cppFunction->arguments(), functionData.arguments);
        QCOMPARE(cppFunction->range(), functionData.range);
    }

    void toFunctionWithLSP_data()
    {
        QTest::addColumn<QString>("fileName");
        QTest::addColumn<QString>("symbolName");
        QTest::addColumn<FunctionData>("functionData");

        QString header = "myobject.h";
        QString source = "myobject.cpp";
        QString main = "main.cpp";
        {
            auto functionData =
                FunctionData {.name = "MyObject::MyObject",
                              .returnType = "", // Constructors don't really return anything
                              .arguments = {Core::Argument {.type = "const std::string &", .name = "message"}},
                              .range = {}};
            QTest::newRow("constructor - header") << header << "MyObject::MyObject" << functionData;
            QTest::newRow("constructor - source") << source << "MyObject::MyObject" << functionData;
        }

        {
            auto functionData = FunctionData {.name = "MyObject::~MyObject",
                                              .returnType = "", // destructors don't return anything
                                              .arguments = {},
                                              .range = {}};
            QTest::newRow("destructor - header") << header << "MyObject::~MyObject" << functionData;
            QTest::newRow("destructor - source") << source << "MyObject::~MyObject" << functionData;
        }

        {
            auto functionData =
                FunctionData {.name = "MyObject::sayMessage", .returnType = "void", .arguments = {}, .range = {}};
            QTest::newRow("member function - header") << header << "MyObject::sayMessage" << functionData;
            QTest::newRow("member function - source") << source << "MyObject::sayMessage" << functionData;
        }

        {
            auto functionData = FunctionData {.name = "main",
                                              .returnType = "int",
                                              .arguments = {Core::Argument {.type = "int", .name = "argc"},
                                                            Core::Argument {.type = "char **", .name = "argv"}},
                                              .range = {}};
            QTest::newRow("free function") << main << "main" << functionData;
        }

        {
            auto functionData = FunctionData {
                .name = "myFreeFunction",
                .returnType = "int",
                .arguments = {Core::Argument {.type = "unsigned int", .name = ""},
                              Core::Argument {.type = "unsigned int", .name = ""},
                              Core::Argument {.type = "long long", .name = ""},
                              Core::Argument {.type = "const std::string", .name = ""},
                              Core::Argument {.type = "const std::string &", .name = ""},
                              Core::Argument {.type = "long long (*)(unsigned int, const std::string &)", .name = ""}},
                .range = {}};
            QTest::newRow("free function with unnamed parameters") << main << "myFreeFunction" << functionData;
        }

        {
            auto functionData = FunctionData {
                .name = "myOtherFreeFunction",
                .returnType = "int",
                .arguments = {Core::Argument {.type = "unsigned int", .name = "a"},
                              Core::Argument {.type = "unsigned int", .name = "b"},
                              Core::Argument {.type = "long long", .name = "c"},
                              Core::Argument {.type = "const std::string", .name = "d"},
                              Core::Argument {.type = "const std::string &", .name = "e_123"},
                              Core::Argument {.type = "long long (*)(unsigned int, const std::string &)", .name = "f"}},
                .range = {}};
            QTest::newRow("free function with complicated named parameters")
                << main << "myOtherFreeFunction" << functionData;
        }
    }

    void toFunctionWithLSP()
    {
        QFETCH(QString, fileName);
        QFETCH(QString, symbolName);
        QFETCH(FunctionData, functionData);

        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/projects/cpp-project");

        auto lspDocument = qobject_cast<Core::LspDocument *>(Core::Project::instance()->open(fileName));
        QVERIFY(lspDocument);

        auto symbol = lspDocument->findSymbol(symbolName);
        QVERIFY(symbol);
        QVERIFY(symbol->isFunction());

        auto fun = symbol->toFunction();
        QVERIFY(fun);

        QCOMPARE(fun->name(), functionData.name);
        QCOMPARE(fun->returnType(), functionData.returnType);

#ifdef OBSOLETE_CLANGD
        QEXPECT_FAIL("constructor - header", "clangd only provides parameter info for clangd 14+", Continue);
        QEXPECT_FAIL("constructor - source", "clangd only provides parameter info for clangd 14+", Continue);
#endif
        QCOMPARE(fun->arguments(), functionData.arguments);
        // do not compare the range here, subject to change in the file, not much sense to testing it.
    }

    void toClass()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/projects/cpp-project");

        auto lspDocument = qobject_cast<Core::LspDocument *>(Core::Project::instance()->open("myobject.h"));
        Core::Symbol *symbol = lspDocument->findSymbol("MyObject");
        QVERIFY(symbol);
        QCOMPARE(symbol->kind(), Core::Symbol::Class);

        Core::ClassSymbol *symbolClass = symbol->toClass();
        QVERIFY(symbolClass);

        QCOMPARE(symbolClass->name(), "MyObject");
        QCOMPARE(symbolClass->members().size(), 10);
        QCOMPARE(symbolClass->members().first()->name(), "MyObject::MyObject");
        QCOMPARE(symbolClass->members().first()->kind(), Core::Symbol::Constructor);
        QCOMPARE(symbolClass->members().at(2)->name(), "MyObject::sayMessage");
        QCOMPARE(symbolClass->members().at(2)->kind(), Core::Symbol::Method);
        QCOMPARE(symbolClass->members().last()->name(), "MyObject::m_enum");
        QCOMPARE(symbolClass->members().last()->kind(), Core::Symbol::Field);
    }

    void references()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/projects/cpp-project");

        auto lspDocument = qobject_cast<Core::LspDocument *>(Core::Project::instance()->open("myobject.h"));
        QVERIFY(lspDocument);

        Core::Symbol *symbol = lspDocument->findSymbol("MyObject");
        QVERIFY(symbol);
        QCOMPARE(symbol->kind(), Core::Symbol::Class);

        const auto isSymbolRange = [&symbol](const auto &loc) {
            return loc.range == symbol->selectionRange();
        };

        const auto references = symbol->references();
        QCOMPARE(references.size(), 9);
        QVERIFY2(std::find_if(references.cbegin(), references.cend(), isSymbolRange) == references.cend(),
                 "Ensure the symbol range itself is not part of the result.");
        QCOMPARE(qobject_cast<Core::LspDocument *>(Core::Project::instance()->currentDocument()), lspDocument);

        for (const auto &reference : references) {
            QVERIFY(reference.document);
        }

        QCOMPARE(std::count_if(references.cbegin(), references.cend(),
                               [](const auto &location) {
                                   return location.document->fileName().endsWith("main.cpp");
                               }),
                 1);

        QCOMPARE(std::count_if(references.cbegin(), references.cend(),
                               [](const auto &location) {
                                   return location.document->fileName().endsWith("myobject.h");
                               }),
                 2);

        QCOMPARE(std::count_if(references.cbegin(), references.cend(),
                               [](const auto &location) {
                                   return location.document->fileName().endsWith("myobject.cpp");
                               }),
                 6);
    }
};

QTEST_MAIN(TestSymbol)
#include "tst_symbol.moc"
