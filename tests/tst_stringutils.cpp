/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "utils/strings.h"

#include <QTest>

using namespace Utils;

class TestStringUtils : public QObject
{
    Q_OBJECT

private slots:
    void test_formatCase()
    {
        QCOMPARE(convertCase("toCamelCase", Utils::Case::CamelCase, Utils::Case::CamelCase), "toCamelCase");
        QCOMPARE(convertCase("ToCamelCase", Utils::Case::PascalCase, Utils::Case::CamelCase), "toCamelCase");
        QCOMPARE(convertCase("to_camel_case", Utils::Case::SnakeCase, Utils::Case::CamelCase), "toCamelCase");
        QCOMPARE(convertCase("to-camel-case", Utils::Case::KebabCase, Utils::Case::CamelCase), "toCamelCase");
        QCOMPARE(convertCase("TO_CAMEL_CASE", Utils::Case::UpperCase, Utils::Case::CamelCase), "toCamelCase");
        QCOMPARE(convertCase("To Camel Case", Utils::Case::TitleCase, Utils::Case::CamelCase), "toCamelCase");

        QCOMPARE(convertCase("toPascalCase", Utils::Case::CamelCase, Utils::Case::PascalCase), "ToPascalCase");
        QCOMPARE(convertCase("ToPascalCase", Utils::Case::PascalCase, Utils::Case::PascalCase), "ToPascalCase");
        QCOMPARE(convertCase("to_pascal_case", Utils::Case::SnakeCase, Utils::Case::PascalCase), "ToPascalCase");
        QCOMPARE(convertCase("to-pascal-case", Utils::Case::KebabCase, Utils::Case::PascalCase), "ToPascalCase");
        QCOMPARE(convertCase("TO_PASCAL_CASE", Utils::Case::UpperCase, Utils::Case::PascalCase), "ToPascalCase");
        QCOMPARE(convertCase("To Pascal Case", Utils::Case::TitleCase, Utils::Case::PascalCase), "ToPascalCase");

        QCOMPARE(convertCase("toSnakeCase", Utils::Case::CamelCase, Utils::Case::SnakeCase), "to_snake_case");
        QCOMPARE(convertCase("ToSnakeCase", Utils::Case::PascalCase, Utils::Case::SnakeCase), "to_snake_case");
        QCOMPARE(convertCase("to_snake_case", Utils::Case::SnakeCase, Utils::Case::SnakeCase), "to_snake_case");
        QCOMPARE(convertCase("to-snake-case", Utils::Case::KebabCase, Utils::Case::SnakeCase), "to_snake_case");
        QCOMPARE(convertCase("TO_SNAKE_CASE", Utils::Case::UpperCase, Utils::Case::SnakeCase), "to_snake_case");
        QCOMPARE(convertCase("To Snake Case", Utils::Case::TitleCase, Utils::Case::SnakeCase), "to_snake_case");

        QCOMPARE(convertCase("toUpperCase", Utils::Case::CamelCase, Utils::Case::UpperCase), "TO_UPPER_CASE");
        QCOMPARE(convertCase("ToUpperCase", Utils::Case::PascalCase, Utils::Case::UpperCase), "TO_UPPER_CASE");
        QCOMPARE(convertCase("to_upper_case", Utils::Case::SnakeCase, Utils::Case::UpperCase), "TO_UPPER_CASE");
        QCOMPARE(convertCase("to-upper-case", Utils::Case::KebabCase, Utils::Case::UpperCase), "TO_UPPER_CASE");
        QCOMPARE(convertCase("TO_UPPER_CASE", Utils::Case::UpperCase, Utils::Case::UpperCase), "TO_UPPER_CASE");
        QCOMPARE(convertCase("To Upper Case", Utils::Case::TitleCase, Utils::Case::UpperCase), "TO_UPPER_CASE");

        QCOMPARE(convertCase("toKebabCase", Utils::Case::CamelCase, Utils::Case::KebabCase), "to-kebab-case");
        QCOMPARE(convertCase("ToKebabCase", Utils::Case::PascalCase, Utils::Case::KebabCase), "to-kebab-case");
        QCOMPARE(convertCase("to_kebab_case", Utils::Case::SnakeCase, Utils::Case::KebabCase), "to-kebab-case");
        QCOMPARE(convertCase("to-kebab-case", Utils::Case::KebabCase, Utils::Case::KebabCase), "to-kebab-case");
        QCOMPARE(convertCase("TO_KEBAB_CASE", Utils::Case::UpperCase, Utils::Case::KebabCase), "to-kebab-case");
        QCOMPARE(convertCase("To Kebab Case", Utils::Case::TitleCase, Utils::Case::KebabCase), "to-kebab-case");

        QCOMPARE(convertCase("toTitleCase", Utils::Case::CamelCase, Utils::Case::TitleCase), "To Title Case");
        QCOMPARE(convertCase("ToTitleCase", Utils::Case::PascalCase, Utils::Case::TitleCase), "To Title Case");
        QCOMPARE(convertCase("to_title_case", Utils::Case::SnakeCase, Utils::Case::TitleCase), "To Title Case");
        QCOMPARE(convertCase("to-title-case", Utils::Case::KebabCase, Utils::Case::TitleCase), "To Title Case");
        QCOMPARE(convertCase("TO_TITLE_CASE", Utils::Case::UpperCase, Utils::Case::TitleCase), "To Title Case");
        QCOMPARE(convertCase("To Title Case", Utils::Case::TitleCase, Utils::Case::TitleCase), "To Title Case");

        QCOMPARE(convertCase("toTitleCaseWithAnException", Utils::Case::CamelCase, Utils::Case::TitleCase),
                 "To Title Case With an Exception");
        QCOMPARE(convertCase("ToTitleCaseWithAnException", Utils::Case::PascalCase, Utils::Case::TitleCase),
                 "To Title Case With an Exception");
        QCOMPARE(convertCase("to_title_case_with_an_exception", Utils::Case::SnakeCase, Utils::Case::TitleCase),
                 "To Title Case With an Exception");
        QCOMPARE(convertCase("to-title-case-with-an-exception", Utils::Case::KebabCase, Utils::Case::TitleCase),
                 "To Title Case With an Exception");
        QCOMPARE(convertCase("TO_TITLE_CASE_WITH_AN_EXCEPTION", Utils::Case::UpperCase, Utils::Case::TitleCase),
                 "To Title Case With an Exception");
        QCOMPARE(convertCase("To Title Case With an Exception", Utils::Case::TitleCase, Utils::Case::TitleCase),
                 "To Title Case With an Exception");
    }

    // This function is copied from from Qt Creator.
    void test_matchCaseReplacement()
    {
        QCOMPARE(matchCaseReplacement("", "foobar"), QString("foobar")); // empty string

        QCOMPARE(matchCaseReplacement("testpad", "foobar"), QString("foobar")); // lower case
        QCOMPARE(matchCaseReplacement("TESTPAD", "foobar"), QString("FOOBAR")); // upper case
        QCOMPARE(matchCaseReplacement("Testpad", "foobar"), QString("Foobar")); // capitalized
        QCOMPARE(matchCaseReplacement("tESTPAD", "foobar"), QString("fOOBAR")); // un-capitalized
        QCOMPARE(matchCaseReplacement("tEsTpAd", "foobar"),
                 QString("foobar")); // mixed case, use replacement as specified
        QCOMPARE(matchCaseReplacement("TeStPaD", "foobar"),
                 QString("foobar")); // mixed case, use replacement as specified

        QCOMPARE(matchCaseReplacement("testpad", "fooBar"), QString("foobar")); // lower case
        QCOMPARE(matchCaseReplacement("TESTPAD", "fooBar"), QString("FOOBAR")); // upper case
        QCOMPARE(matchCaseReplacement("Testpad", "fooBar"), QString("Foobar")); // capitalized
        QCOMPARE(matchCaseReplacement("tESTPAD", "fooBar"), QString("fOOBAR")); // un-capitalized
        QCOMPARE(matchCaseReplacement("tEsTpAd", "fooBar"),
                 QString("fooBar")); // mixed case, use replacement as specified
        QCOMPARE(matchCaseReplacement("TeStPaD", "fooBar"),
                 QString("fooBar")); // mixed case, use replacement as specified

        // with common prefix
        QCOMPARE(matchCaseReplacement("pReFiXtestpad", "prefixfoobar"), QString("pReFiXfoobar")); // lower case
        QCOMPARE(matchCaseReplacement("pReFiXTESTPAD", "prefixfoobar"), QString("pReFiXFOOBAR")); // upper case
        QCOMPARE(matchCaseReplacement("pReFiXTestpad", "prefixfoobar"), QString("pReFiXFoobar")); // capitalized
        QCOMPARE(matchCaseReplacement("pReFiXtESTPAD", "prefixfoobar"), QString("pReFiXfOOBAR")); // un-capitalized
        QCOMPARE(matchCaseReplacement("pReFiXtEsTpAd", "prefixfoobar"),
                 QString("pReFiXfoobar")); // mixed case, use replacement as specified
        QCOMPARE(matchCaseReplacement("pReFiXTeStPaD", "prefixfoobar"),
                 QString("pReFiXfoobar")); // mixed case, use replacement as specified

        // with common suffix
        QCOMPARE(matchCaseReplacement("testpadSuFfIx", "foobarsuffix"), QString("foobarSuFfIx")); // lower case
        QCOMPARE(matchCaseReplacement("TESTPADSuFfIx", "foobarsuffix"), QString("FOOBARSuFfIx")); // upper case
        QCOMPARE(matchCaseReplacement("TestpadSuFfIx", "foobarsuffix"), QString("FoobarSuFfIx")); // capitalized
        QCOMPARE(matchCaseReplacement("tESTPADSuFfIx", "foobarsuffix"), QString("fOOBARSuFfIx")); // un-capitalized
        QCOMPARE(matchCaseReplacement("tEsTpAdSuFfIx", "foobarsuffix"),
                 QString("foobarSuFfIx")); // mixed case, use replacement as specified
        QCOMPARE(matchCaseReplacement("TeStPaDSuFfIx", "foobarsuffix"),
                 QString("foobarSuFfIx")); // mixed case, use replacement as specified

        // with common prefix and suffix
        QCOMPARE(matchCaseReplacement("pReFiXtestpadSuFfIx", "prefixfoobarsuffix"),
                 QString("pReFiXfoobarSuFfIx")); // lower case
        QCOMPARE(matchCaseReplacement("pReFiXTESTPADSuFfIx", "prefixfoobarsuffix"),
                 QString("pReFiXFOOBARSuFfIx")); // upper case
        QCOMPARE(matchCaseReplacement("pReFiXTestpadSuFfIx", "prefixfoobarsuffix"),
                 QString("pReFiXFoobarSuFfIx")); // capitalized
        QCOMPARE(matchCaseReplacement("pReFiXtESTPADSuFfIx", "prefixfoobarsuffix"),
                 QString("pReFiXfOOBARSuFfIx")); // un-capitalized
        QCOMPARE(matchCaseReplacement("pReFiXtEsTpAdSuFfIx", "prefixfoobarsuffix"),
                 QString("pReFiXfoobarSuFfIx")); // mixed case, use replacement as specified
        QCOMPARE(matchCaseReplacement("pReFiXTeStPaDSuFfIx", "prefixfoobarsuffix"),
                 QString("pReFiXfoobarSuFfIx")); // mixed case, use replacement as specified
    }
};

QTEST_APPLESS_MAIN(TestStringUtils)

#include "tst_stringutils.moc"
