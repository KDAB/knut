/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick
import Knut
import Knut.Test

TestCase {
    name: "Utils"

    function test_global() {
        Utils.setGlobal("foo", "bar")
        compare(Utils.getGlobal("foo"), "bar");
        compare(Utils.getGlobal("baz"), "")
    }

    function test_convertCase() {
        compare(Utils.convertCase("ToCamelCase", Utils.PascalCase, Utils.CamelCase), "toCamelCase")
        compare(Utils.convertCase("to_camel_case", Utils.SnakeCase, Utils.CamelCase), "toCamelCase")
        compare(Utils.convertCase("to-camel-case", Utils.KebabCase, Utils.CamelCase), "toCamelCase")
        compare(Utils.convertCase("TO_CAMEL_CASE", Utils.UpperCase, Utils.CamelCase), "toCamelCase")
        compare(Utils.convertCase("To Camel Case", Utils.TitleCase, Utils.CamelCase), "toCamelCase")

        compare(Utils.convertCase("toPascalCase", Utils.CamelCase, Utils.PascalCase), "ToPascalCase")
        compare(Utils.convertCase("to_pascal_case", Utils.SnakeCase, Utils.PascalCase), "ToPascalCase")
        compare(Utils.convertCase("to-pascal-case", Utils.KebabCase, Utils.PascalCase), "ToPascalCase")
        compare(Utils.convertCase("TO_PASCAL_CASE", Utils.UpperCase, Utils.PascalCase), "ToPascalCase")
        compare(Utils.convertCase("To Pascal Case", Utils.TitleCase, Utils.PascalCase), "ToPascalCase")

        compare(Utils.convertCase("toSnakeCase", Utils.CamelCase, Utils.SnakeCase), "to_snake_case")
        compare(Utils.convertCase("ToSnakeCase", Utils.PascalCase, Utils.SnakeCase), "to_snake_case")
        compare(Utils.convertCase("to-snake-case", Utils.KebabCase, Utils.SnakeCase), "to_snake_case")
        compare(Utils.convertCase("TO_SNAKE_CASE", Utils.UpperCase, Utils.SnakeCase), "to_snake_case")
        compare(Utils.convertCase("To Snake Case", Utils.TitleCase, Utils.SnakeCase), "to_snake_case")

        compare(Utils.convertCase("toUpperCase", Utils.CamelCase, Utils.UpperCase), "TO_UPPER_CASE")
        compare(Utils.convertCase("ToUpperCase", Utils.PascalCase, Utils.UpperCase), "TO_UPPER_CASE")
        compare(Utils.convertCase("to_upper_case", Utils.SnakeCase, Utils.UpperCase), "TO_UPPER_CASE")
        compare(Utils.convertCase("to-upper-case", Utils.KebabCase, Utils.UpperCase), "TO_UPPER_CASE")
        compare(Utils.convertCase("To Upper Case", Utils.TitleCase, Utils.UpperCase), "TO_UPPER_CASE")

        compare(Utils.convertCase("toKebabCase", Utils.CamelCase, Utils.KebabCase), "to-kebab-case")
        compare(Utils.convertCase("ToKebabCase", Utils.PascalCase, Utils.KebabCase), "to-kebab-case")
        compare(Utils.convertCase("to_kebab_case", Utils.SnakeCase, Utils.KebabCase), "to-kebab-case")
        compare(Utils.convertCase("TO_KEBAB_CASE", Utils.UpperCase, Utils.KebabCase), "to-kebab-case")
        compare(Utils.convertCase("To Kebab Case", Utils.TitleCase, Utils.KebabCase), "to-kebab-case")

        compare(Utils.convertCase("toTitleCase", Utils.CamelCase, Utils.TitleCase), "To Title Case")
        compare(Utils.convertCase("ToTitleCase", Utils.PascalCase, Utils.TitleCase), "To Title Case")
        compare(Utils.convertCase("to_title_case", Utils.SnakeCase, Utils.TitleCase), "To Title Case")
        compare(Utils.convertCase("to-title-case", Utils.KebabCase, Utils.TitleCase), "To Title Case")
        compare(Utils.convertCase("TO_TITLE_CASE", Utils.UpperCase, Utils.TitleCase), "To Title Case")

        compare(Utils.convertCase("toTitleCaseWithAnException", Utils.CamelCase, Utils.TitleCase), "To Title Case With an Exception")
        compare(Utils.convertCase("ToTitleCaseWithAnException", Utils.PascalCase, Utils.TitleCase), "To Title Case With an Exception")
        compare(Utils.convertCase("to_title_case_with_an_exception", Utils.SnakeCase, Utils.TitleCase), "To Title Case With an Exception")
        compare(Utils.convertCase("to-title-case-with-an-exception", Utils.KebabCase, Utils.TitleCase), "To Title Case With an Exception")
        compare(Utils.convertCase("TO_TITLE_CASE_WITH_AN_EXCEPTION", Utils.UpperCase, Utils.TitleCase), "To Title Case With an Exception")
    }

    function test_cppKeywords() {
        var keywords = Utils.cppKeywords()
        verify(keywords.length > 0)
        verify(keywords.includes("delete"))
    }

    function test_cppPrimitiveTypes() {
        var primitiveTypes = Utils.cppPrimitiveTypes()
        verify(primitiveTypes.length > 0)
        verify(primitiveTypes.includes("wchar_t"))
    }
}
