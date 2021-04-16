#include "json_utils.h"

#include <doctest/doctest.h>

#include <string>

using json = nlohmann::json;

///////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////
enum class Color { Red, Green, Blue };

struct TestStruct
{
    struct InnerStruct
    {
        std::optional<bool> boolValue;
        std::string stringValue;
    };

    InnerStruct inner;
    QString string;
    QStringList stringList;
    std::variant<int, std::string> intOrString;
    std::variant<int, std::nullptr_t> intOrNull;
    std::variant<std::nullptr_t, int> nullOrInt;
    Color color;
    std::optional<Color> optColor;
    std::optional<std::variant<std::string, int>> optVariant;
};
JSONIFY(TestStruct::InnerStruct, boolValue, stringValue);
JSONIFY(TestStruct, string, stringList, intOrString, intOrNull, nullOrInt, color, optColor, optVariant, inner);

bool operator==(const TestStruct &lhs, const TestStruct &rhs)
{
    return lhs.inner.boolValue == rhs.inner.boolValue && lhs.inner.stringValue == rhs.inner.stringValue
        && lhs.string == rhs.string && lhs.stringList == rhs.stringList && lhs.intOrString == rhs.intOrString
        && lhs.intOrNull == rhs.intOrNull && lhs.nullOrInt == rhs.nullOrInt && lhs.color == rhs.color
        && lhs.optColor == rhs.optColor && lhs.optVariant == rhs.optVariant;
}

TEST_SUITE("utils")
{
    TEST_CASE("json")
    {
        TestStruct test;
        test.inner.boolValue = true;
        test.inner.stringValue = "innerText";
        test.string = "QString";
        test.stringList = QStringList {"one", "two", "three"};
        test.intOrString = 10;
        test.intOrNull = 20;
        test.nullOrInt = nullptr;
        test.color = Color::Blue;
        test.optColor = Color::Green;
        test.optVariant = "variantText";

        TestStruct test2;
        test2.intOrString = "string";
        test2.intOrNull = nullptr;
        test2.nullOrInt = 10;
        test2.color = Color::Red;

        SUBCASE("serialize")
        {
            json j = test;
            j.dump();

            CHECK_EQ(
                j.dump(),
                R"({"color":"blue","inner":{"boolValue":true,"stringValue":"innerText"},"intOrNull":20,"intOrString":10,"nullOrInt":null,"optColor":"green","optVariant":"variantText","string":"QString","stringList":["one","two","three"]})");
            CHECK(j["intOrString"].is_number_integer());
            CHECK(j["intOrNull"].is_number_integer());
            CHECK(j["nullOrInt"].is_null());

            json j2 = test2;
            j2.dump();

            CHECK_EQ(
                j2.dump(),
                R"({"color":"red","inner":{"stringValue":""},"intOrNull":null,"intOrString":"string","nullOrInt":10,"string":"","stringList":[]})");
            CHECK(j2["intOrString"].is_string());
            CHECK(j2["intOrNull"].is_null());
            CHECK(j2["nullOrInt"].is_number_integer());
        }
        SUBCASE("deserialize")
        {
            json j = json::parse(
                R"({"color":"blue","inner":{"boolValue":true,"stringValue":"innerText"},"intOrNull":20,"intOrString":10,"nullOrInt":null,"optColor":"green","optVariant":"variantText","string":"QString","stringList":["one","two","three"]})");
            auto result = j.get<TestStruct>();

            CHECK_EQ(result, test);

            json j2 = json::parse(
                R"({"color":"red","inner":{"stringValue":""},"intOrNull":null,"intOrString":"string","nullOrInt":10,"string":"","stringList":[]})");
            auto result2 = j2.get<TestStruct>();

            CHECK_EQ(result2, test2);
        }
    }
}
