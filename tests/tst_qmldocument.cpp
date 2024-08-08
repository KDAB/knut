/*
 This file is part of Knut.

 SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

 SPDX-License-Identifier: GPL-3.0-only

 Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_cpputils.h"
#include "common/test_utils.h"
#include "core/cppdocument.h"
#include "core/knutcore.h"
#include "core/symbol.h"
#include "core/utils.h"
#include <algorithm>
#include <array>

class TestQmlDocument : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }
    void testAllSymbols()
    {
        Test::testQmlDocument("", "tst_symbols.qml", [](auto *document) {
            auto allSymbols = document->symbols();
            QCOMPARE(allSymbols.size(), 18);

            auto isUIObject = [](auto *sym) {
                return sym->kind() == Core::Symbol::Kind::Object;
            };
            const auto uiObjectDefinitions = std::ranges::count_if(allSymbols, isUIObject);
            QCOMPARE(uiObjectDefinitions, 8);

            auto isFunction = [](auto *sym) {
                return sym->kind() == Core::Symbol::Kind::Function;
            };
            const auto functionSymbols = std::ranges::count_if(allSymbols, isFunction);
            QCOMPARE(functionSymbols, 3);

            auto isProperty = [](auto *sym) {
                return sym->kind() == Core::Symbol::Kind::Field;
            };
            const auto propSymbols = std::ranges::count_if(allSymbols, isProperty);
            QCOMPARE(propSymbols, 7);
        });

        Test::testQmlDocument("", "tst_symbols.qml", [](auto *document) {
            std::array uiObjectNames = {"TestCase", "TestCase::sub::Another", "TestCase::Standalone",
                                        "TestCase::Standalone::NestedA", "NestedB"};
            for (auto &name : uiObjectNames) {
                auto TestCase = document->findSymbol(name);
                QVERIFY(TestCase);
                QCOMPARE(TestCase->kind(), Core::Symbol::Object);
            }

            std::array fieldNames = {
                "TestCase::id",
                "TestCase::age",
                "TestCase::name",
                "TestCase::sub::Another::id",
            };

            for (auto &name : fieldNames) {
                auto sym = document->findSymbol(name);
                QVERIFY(sym);
                QCOMPARE(sym->kind(), Core::Symbol::Field);
            }
        });
    }
};

QTEST_MAIN(TestQmlDocument)
#include "tst_qmldocument.moc"
