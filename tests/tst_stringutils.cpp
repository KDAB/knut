#include <QtTest>

#include "core/string_utils.h"

using namespace Core;

class TestStringUtils : public QObject
{
    Q_OBJECT

private slots:
    void test_formatCase()
    {
        QCOMPARE(convertCase("toCamelCase", Core::Case::CamelCase, Core::Case::CamelCase), "toCamelCase");
        QCOMPARE(convertCase("ToCamelCase", Core::Case::PascalCase, Core::Case::CamelCase), "toCamelCase");
        QCOMPARE(convertCase("to_camel_case", Core::Case::SnakeCase, Core::Case::CamelCase), "toCamelCase");
        QCOMPARE(convertCase("to-camel-case", Core::Case::KebabCase, Core::Case::CamelCase), "toCamelCase");
        QCOMPARE(convertCase("TO_CAMEL_CASE", Core::Case::UpperCase, Core::Case::CamelCase), "toCamelCase");
        QCOMPARE(convertCase("To Camel Case", Core::Case::TitleCase, Core::Case::CamelCase), "toCamelCase");

        QCOMPARE(convertCase("toPascalCase", Core::Case::CamelCase, Core::Case::PascalCase), "ToPascalCase");
        QCOMPARE(convertCase("ToPascalCase", Core::Case::PascalCase, Core::Case::PascalCase), "ToPascalCase");
        QCOMPARE(convertCase("to_pascal_case", Core::Case::SnakeCase, Core::Case::PascalCase), "ToPascalCase");
        QCOMPARE(convertCase("to-pascal-case", Core::Case::KebabCase, Core::Case::PascalCase), "ToPascalCase");
        QCOMPARE(convertCase("TO_PASCAL_CASE", Core::Case::UpperCase, Core::Case::PascalCase), "ToPascalCase");
        QCOMPARE(convertCase("To Pascal Case", Core::Case::TitleCase, Core::Case::PascalCase), "ToPascalCase");

        QCOMPARE(convertCase("toSnakeCase", Core::Case::CamelCase, Core::Case::SnakeCase), "to_snake_case");
        QCOMPARE(convertCase("ToSnakeCase", Core::Case::PascalCase, Core::Case::SnakeCase), "to_snake_case");
        QCOMPARE(convertCase("to_snake_case", Core::Case::SnakeCase, Core::Case::SnakeCase), "to_snake_case");
        QCOMPARE(convertCase("to-snake-case", Core::Case::KebabCase, Core::Case::SnakeCase), "to_snake_case");
        QCOMPARE(convertCase("TO_SNAKE_CASE", Core::Case::UpperCase, Core::Case::SnakeCase), "to_snake_case");
        QCOMPARE(convertCase("To Snake Case", Core::Case::TitleCase, Core::Case::SnakeCase), "to_snake_case");

        QCOMPARE(convertCase("toUpperCase", Core::Case::CamelCase, Core::Case::UpperCase), "TO_UPPER_CASE");
        QCOMPARE(convertCase("ToUpperCase", Core::Case::PascalCase, Core::Case::UpperCase), "TO_UPPER_CASE");
        QCOMPARE(convertCase("to_upper_case", Core::Case::SnakeCase, Core::Case::UpperCase), "TO_UPPER_CASE");
        QCOMPARE(convertCase("to-upper-case", Core::Case::KebabCase, Core::Case::UpperCase), "TO_UPPER_CASE");
        QCOMPARE(convertCase("TO_UPPER_CASE", Core::Case::UpperCase, Core::Case::UpperCase), "TO_UPPER_CASE");
        QCOMPARE(convertCase("To Upper Case", Core::Case::TitleCase, Core::Case::UpperCase), "TO_UPPER_CASE");

        QCOMPARE(convertCase("toKebabCase", Core::Case::CamelCase, Core::Case::KebabCase), "to-kebab-case");
        QCOMPARE(convertCase("ToKebabCase", Core::Case::PascalCase, Core::Case::KebabCase), "to-kebab-case");
        QCOMPARE(convertCase("to_kebab_case", Core::Case::SnakeCase, Core::Case::KebabCase), "to-kebab-case");
        QCOMPARE(convertCase("to-kebab-case", Core::Case::KebabCase, Core::Case::KebabCase), "to-kebab-case");
        QCOMPARE(convertCase("TO_KEBAB_CASE", Core::Case::UpperCase, Core::Case::KebabCase), "to-kebab-case");
        QCOMPARE(convertCase("To Kebab Case", Core::Case::TitleCase, Core::Case::KebabCase), "to-kebab-case");

        QCOMPARE(convertCase("toTitleCase", Core::Case::CamelCase, Core::Case::TitleCase), "To Title Case");
        QCOMPARE(convertCase("ToTitleCase", Core::Case::PascalCase, Core::Case::TitleCase), "To Title Case");
        QCOMPARE(convertCase("to_title_case", Core::Case::SnakeCase, Core::Case::TitleCase), "To Title Case");
        QCOMPARE(convertCase("to-title-case", Core::Case::KebabCase, Core::Case::TitleCase), "To Title Case");
        QCOMPARE(convertCase("TO_TITLE_CASE", Core::Case::UpperCase, Core::Case::TitleCase), "To Title Case");
        QCOMPARE(convertCase("To Title Case", Core::Case::TitleCase, Core::Case::TitleCase), "To Title Case");

        QCOMPARE(convertCase("toTitleCaseWithAnException", Core::Case::CamelCase, Core::Case::TitleCase),
                 "To Title Case With an Exception");
        QCOMPARE(convertCase("ToTitleCaseWithAnException", Core::Case::PascalCase, Core::Case::TitleCase),
                 "To Title Case With an Exception");
        QCOMPARE(convertCase("to_title_case_with_an_exception", Core::Case::SnakeCase, Core::Case::TitleCase),
                 "To Title Case With an Exception");
        QCOMPARE(convertCase("to-title-case-with-an-exception", Core::Case::KebabCase, Core::Case::TitleCase),
                 "To Title Case With an Exception");
        QCOMPARE(convertCase("TO_TITLE_CASE_WITH_AN_EXCEPTION", Core::Case::UpperCase, Core::Case::TitleCase),
                 "To Title Case With an Exception");
        QCOMPARE(convertCase("To Title Case With an Exception", Core::Case::TitleCase, Core::Case::TitleCase),
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
