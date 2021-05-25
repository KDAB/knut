#include <QtTest>

#include "core/string_utils.h"

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
};

QTEST_APPLESS_MAIN(TestStringUtils)

#include "tst_stringutils.moc"
