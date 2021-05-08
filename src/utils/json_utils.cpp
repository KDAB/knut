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
    std::variant<int, std::nullptr_t> intOrNull;
    std::variant<std::nullptr_t, int> nullOrInt;
    std::variant<int, std::string, std::nullptr_t> id;
    using ValueOrList = std::variant<QString, QStringList>;
    ValueOrList valueOrList;
    Color color;
    std::optional<Color> optColor;
    std::optional<std::variant<std::string, int>> optVariant;
};
JSONIFY(TestStruct::InnerStruct, boolValue, stringValue);
JSONIFY(TestStruct, string, stringList, intOrNull, nullOrInt, id, valueOrList, color, optColor, optVariant, inner);

struct SmallStruct
{
    QString string;
    QStringList stringList;
    Color color;
};
JSONIFY(SmallStruct, string, stringList, color);

struct EmptyStruct
{
};
JSONIFY_EMPTY(EmptyStruct)

bool operator==(const TestStruct &lhs, const TestStruct &rhs)
{
    return lhs.inner.boolValue == rhs.inner.boolValue && lhs.inner.stringValue == rhs.inner.stringValue
        && lhs.string == rhs.string && lhs.stringList == rhs.stringList && lhs.intOrNull == rhs.intOrNull
        && lhs.nullOrInt == rhs.nullOrInt && lhs.id == rhs.id && lhs.valueOrList == rhs.valueOrList
        && lhs.color == rhs.color && lhs.optColor == rhs.optColor && lhs.optVariant == rhs.optVariant;
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
        test.intOrNull = 20;
        test.nullOrInt = nullptr;
        test.id = 10;
        test.valueOrList = "test";
        test.color = Color::Blue;
        test.optColor = Color::Green;
        test.optVariant = "variantText";

        TestStruct test2;
        test2.intOrNull = nullptr;
        test2.nullOrInt = 10;
        test2.id = "string";
        test2.valueOrList = QStringList {"one", "two", "three"};
        test2.color = Color::Red;

        SUBCASE("serialize")
        {
            json j = test;
            j.dump();

            CHECK_EQ(
                j.dump(),
                R"({"color":"blue","id":10,"inner":{"boolValue":true,"stringValue":"innerText"},"intOrNull":20,"nullOrInt":null,"optColor":"green","optVariant":"variantText","string":"QString","stringList":["one","two","three"],"valueOrList":"test"})");
            CHECK(j["intOrNull"].is_number_integer());
            CHECK(j["nullOrInt"].is_null());
            CHECK(j["id"].is_number_integer());
            auto value = j["valueOrList"].get<TestStruct::ValueOrList>();
            CHECK_EQ(std::get<QString>(value), QString("test"));

            json j2 = test2;
            j2.dump();

            CHECK_EQ(
                j2.dump(),
                R"({"color":"red","id":"string","inner":{"stringValue":""},"intOrNull":null,"nullOrInt":10,"string":"","stringList":[],"valueOrList":["one","two","three"]})");
            CHECK(j2["intOrNull"].is_null());
            CHECK(j2["nullOrInt"].is_number_integer());
            CHECK(j2["id"].is_string());
            auto value2 = j2["valueOrList"].get<TestStruct::ValueOrList>();
            CHECK_EQ(std::get<QStringList>(value2), QStringList {"one", "two", "three"});
        }

        SUBCASE("deserialize")
        {
            json j = json::parse(
                R"({"color":"blue","id":10,"inner":{"boolValue":true,"stringValue":"innerText"},"intOrNull":20,"nullOrInt":null,"optColor":"green","optVariant":"variantText","string":"QString","stringList":["one","two","three"],"valueOrList":"test"})");
            auto result = j.get<TestStruct>();

            CHECK_EQ(result, test);

            json j2 = json::parse(
                R"({"color":"red","id":"string","inner":{"stringValue":""},"intOrNull":null,"nullOrInt":10,"string":"","stringList":[],"valueOrList":["one","two","three"]})");
            auto result2 = j2.get<TestStruct>();

            CHECK_EQ(result2, test2);
        }

        SUBCASE("empty struct")
        {
            EmptyStruct empty;
            json j = empty;
            CHECK_EQ(j.dump(), "{}");

            json j2 = json::parse("{}");
            CHECK_NOTHROW(j.get<EmptyStruct>());
        }

        SUBCASE("wrong deserialization")
        {
            json j = json::parse(R"({"string":1,"stringList":[],"color":"red"})");
            CHECK_THROWS_AS(j.get<SmallStruct>(), nlohmann::detail::type_error);

            json j2 = json::parse(R"({"string":"test","stringList":"wrong","color":"red"})");
            CHECK_THROWS_AS(j2.get<SmallStruct>(), nlohmann::detail::type_error);

            json j3 = json::parse(R"({"string":"test","stringList":[],"color":1})");
            CHECK_THROWS_AS(j3.get<SmallStruct>(), nlohmann::detail::type_error);
        }
    }
}
