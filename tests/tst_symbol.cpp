/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"
#include "core/classsymbol.h"
#include "core/codedocument.h"
#include "core/functionsymbol.h"
#include "core/knutcore.h"
#include "core/project.h"
#include "core/symbol.h"
#include "core/typedsymbol.h"

#include <QTest>
#include <QThread>

namespace Core {

char *toString(const FunctionArgument &argument)
{
    return QTest::toString(QString("Type: '%1' Name: '%2'").arg(argument.type, argument.name));
}

}

class TestSymbol : public QObject
{
    Q_OBJECT

    struct FunctionData
    {
        QString name;
        QString returnType;
        QList<Core::FunctionArgument> arguments;
        Core::RangeMark range;

        bool isNull() { return name.isEmpty(); }
    };

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

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
                              .arguments = {Core::FunctionArgument {.type = "const std::string&", .name = "message"}},
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
            auto functionData =
                FunctionData {.name = "main",
                              .returnType = "int",
                              .arguments = {Core::FunctionArgument {.type = "int", .name = "argc"},
                                            Core::FunctionArgument {.type = "char *[]", .name = "argv"}},
                              .range = {}};
            QTest::newRow("free function") << main << "main" << functionData;
        }

        {
            auto functionData =
                FunctionData {.name = "myFreeFunction",
                              .returnType = "std::string",
                              .arguments = {Core::FunctionArgument {.type = "unsigned", .name = ""},
                                            Core::FunctionArgument {.type = "unsigned int", .name = ""},
                                            Core::FunctionArgument {.type = "long long", .name = ""},
                                            Core::FunctionArgument {.type = "const string", .name = ""},
                                            Core::FunctionArgument {.type = "const std::string&", .name = ""},
                                            Core::FunctionArgument {
                                                .type = "long long (*)(unsigned, const std::string&)", .name = ""}},
                              .range = {}};
            QTest::newRow("free function with unnamed parameters") << main << "myFreeFunction" << functionData;
        }

        {
            auto functionData =
                FunctionData {.name = "myOtherFreeFunction",
                              .returnType = "int",
                              .arguments = {Core::FunctionArgument {.type = "unsigned", .name = "a"},
                                            Core::FunctionArgument {.type = "unsigned int", .name = "b"},
                                            Core::FunctionArgument {.type = "long long", .name = "c"},
                                            Core::FunctionArgument {.type = "const string", .name = "d"},
                                            Core::FunctionArgument {.type = "const std::string&", .name = "e_123"},
                                            Core::FunctionArgument {
                                                .type = "long long (*)(unsigned, const std::string&)", .name = "f"}},
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

        auto codeDocument = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->open(fileName));
        QVERIFY(codeDocument);

        auto symbol = codeDocument->findSymbol(symbolName);
        QVERIFY(symbol);
        QVERIFY(symbol->isFunction());

        auto fun = symbol->toFunction();
        QVERIFY(fun);

        QCOMPARE(fun->name(), functionData.name);
        // Some version of clang adds the "std::", some don't...
        QCOMPARE(fun->returnType().remove("std::"), functionData.returnType.remove("std::"));

        auto args = fun->arguments();
        QCOMPARE(args, functionData.arguments);
        // do not compare the range here, subject to change in the file, not much sense to testing it.
    }

    void toClass()
    {
        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/projects/cpp-project");

        auto codeDocument = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->open("myobject.h"));
        Core::Symbol *symbol = codeDocument->findSymbol("MyObject");
        QVERIFY(symbol);
        QCOMPARE(symbol->kind(), Core::Symbol::Class);

        Core::ClassSymbol *symbolClass = symbol->toClass();
        QVERIFY(symbolClass);

        QCOMPARE(symbolClass->name(), "MyObject");
        const auto members = symbolClass->members();
        QCOMPARE(members.size(), 10);
        QCOMPARE(members.first()->name(), "MyObject::MyObject");
        QCOMPARE(members.first()->kind(), Core::Symbol::Constructor);
        QCOMPARE(members.at(2)->name(), "MyObject::sayMessage");
        QCOMPARE(members.at(2)->kind(), Core::Symbol::Method);
        QCOMPARE(members.last()->name(), "MyObject::m_enum");
        QCOMPARE(members.last()->kind(), Core::Symbol::Field);

        auto typedsymbol = qobject_cast<Core::TypedSymbol *>(members.at(8));
        QVERIFY(typedsymbol);
        QCOMPARE(typedsymbol->type(), "std::string");

        typedsymbol = qobject_cast<Core::TypedSymbol *>(members.last());
        QVERIFY(typedsymbol);
        QCOMPARE(typedsymbol->type(), "MyEnum");
    }

    void references()
    {
        QSKIP("Skip test: Symbol::references is deprecated.");
        CHECK_CLANGD;

        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/projects/cpp-project");

        auto codeDocument = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->open("myobject.h"));
        QVERIFY(codeDocument);

        spdlog::warn("Finding symbol");
        Core::Symbol *symbol = codeDocument->findSymbol("MyObject");
        QVERIFY(symbol);
        QCOMPARE(symbol->kind(), Core::Symbol::Class);

        const auto isSymbolRange = [&symbol](const auto &loc) {
            return loc == symbol->selectionRange();
        };

        spdlog::warn("Finding references");
        const auto references = symbol->references();
        QCOMPARE(references.size(), 9);
        QVERIFY2(std::ranges::find_if(references, isSymbolRange) == references.cend(),
                 "Ensure the symbol range itself is not part of the result.");
        QCOMPARE(qobject_cast<Core::CodeDocument *>(Core::Project::instance()->currentDocument()), codeDocument);

        spdlog::warn("Verifying document existence");
        for (const auto &reference : references) {
            QVERIFY(reference.document());
        }

        spdlog::warn("Counting documents");
        QCOMPARE(std::ranges::count_if(references,
                                       [](const auto &location) {
                                           return location.document()->fileName().endsWith("main.cpp");
                                       }),
                 1);

        QCOMPARE(std::ranges::count_if(references,
                                       [](const auto &location) {
                                           return location.document()->fileName().endsWith("myobject.h");
                                       }),
                 2);

        QCOMPARE(std::ranges::count_if(references,
                                       [](const auto &location) {
                                           return location.document()->fileName().endsWith("myobject.cpp");
                                       }),
                 6);
    }

    void typedSymbol()
    {

        Core::KnutCore core;
        Core::Project::instance()->setRoot(Test::testDataPath() + "/tst_symbol/");

        auto codeDocument = qobject_cast<Core::CodeDocument *>(Core::Project::instance()->open("typedsymbol.h"));
        QVERIFY(codeDocument);

        auto testTypedSymbol = [&codeDocument](const QString &symbolName, const QString &expectedType) {
            auto symbol = codeDocument->findSymbol(symbolName);
            QVERIFY(symbol);
            auto typedSymbol = qobject_cast<Core::TypedSymbol *>(symbol);
            QVERIFY(typedSymbol);
            QCOMPARE(typedSymbol->type(), expectedType);
        };

        testTypedSymbol("TestClass::m_ptr", "void *");
        testTypedSymbol("TestClass::m_lvalue_reference", "const std::string &");
        testTypedSymbol("TestClass::m_rvalue_reference", "std::string&&");
        testTypedSymbol("TestClass::m_const_ptr", "const class T*const");
    }
};

QTEST_MAIN(TestSymbol)
#include "tst_symbol.moc"
