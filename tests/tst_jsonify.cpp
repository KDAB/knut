#include "core/json_utils.h"
#include "lsp/json.h"

#include <QSignalSpy>
#include <QTest>

#include <string>

using json = nlohmann::json;

///////////////////////////////////////////////////////////////////////////////
// Tests Data
///////////////////////////////////////////////////////////////////////////////
enum class Color { Red, Green, Blue };
JSONIFY_ENUM(Color,
             {
                 {Color::Red, "red"},
                 {Color::Green, "green"},
                 {Color::Blue, "blue"},
             })

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
};
JSONIFY(SmallStruct, string, stringList);

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
bool operator==(const EmptyStruct &, const EmptyStruct &)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////
class TestJsonify : public QObject
{
    Q_OBJECT

    TestStruct m_test;
    TestStruct m_test2;

private slots:
    void initTestCase()
    {
        m_test.inner.boolValue = true;
        m_test.inner.stringValue = "innerText";
        m_test.string = "QString";
        m_test.stringList = QStringList {"one", "two", "three"};
        m_test.intOrNull = 20;
        m_test.nullOrInt = nullptr;
        m_test.id = 10;
        m_test.valueOrList = "test";
        m_test.color = Color::Blue;
        m_test.optColor = Color::Green;
        m_test.optVariant = "variantText";

        m_test2.intOrNull = nullptr;
        m_test2.nullOrInt = 10;
        m_test2.id = "string";
        m_test2.valueOrList = QStringList {"one", "two", "three"};
        m_test2.color = Color::Red;
    }

    void serialize()
    {
        json j = m_test;
        j.dump();

        QCOMPARE(
            j.dump(),
            R"({"color":"blue","id":10,"inner":{"boolValue":true,"stringValue":"innerText"},"intOrNull":20,"nullOrInt":null,"optColor":"green","optVariant":"variantText","string":"QString","stringList":["one","two","three"],"valueOrList":"test"})");
        QVERIFY(j["intOrNull"].is_number_integer());
        QVERIFY(j["nullOrInt"].is_null());
        QVERIFY(j["id"].is_number_integer());
        auto value = j["valueOrList"].get<TestStruct::ValueOrList>();
        QCOMPARE(std::get<QString>(value), QString("test"));

        json j2 = m_test2;
        j2.dump();

        QCOMPARE(
            j2.dump(),
            R"({"color":"red","id":"string","inner":{"stringValue":""},"intOrNull":null,"nullOrInt":10,"string":"","stringList":[],"valueOrList":["one","two","three"]})");
        QVERIFY(j2["intOrNull"].is_null());
        QVERIFY(j2["nullOrInt"].is_number_integer());
        QVERIFY(j2["id"].is_string());
        auto value2 = j2["valueOrList"].get<TestStruct::ValueOrList>();
        QCOMPARE(std::get<QStringList>(value2), (QStringList {"one", "two", "three"}));
    }

    void deserialize()
    {
        json j = json::parse(
            R"({"color":"blue","id":10,"inner":{"boolValue":true,"stringValue":"innerText"},"intOrNull":20,"nullOrInt":null,"optColor":"green","optVariant":"variantText","string":"QString","stringList":["one","two","three"],"valueOrList":"test"})");
        auto result = j.get<TestStruct>();

        QCOMPARE(result, m_test);

        json j2 = json::parse(
            R"({"color":"red","id":"string","inner":{"stringValue":""},"intOrNull":null,"nullOrInt":10,"string":"","stringList":[],"valueOrList":["one","two","three"]})");
        auto result2 = j2.get<TestStruct>();

        QCOMPARE(result2, m_test2);
    }

    void emptyStruct()
    {
        EmptyStruct empty;
        json j = empty;
        QCOMPARE(j.dump(), "{}");

        json j2 = json::parse("{}");
        QCOMPARE(j.get<EmptyStruct>(), empty);
    }

    void throwError()
    {
        json j = json::parse(R"({"string":1,"stringList":[]})");
        QVERIFY_EXCEPTION_THROWN(j.get<SmallStruct>(), nlohmann::detail::type_error);

        json j2 = json::parse(R"({"string":"test","stringList":"wrong"})");
        QVERIFY_EXCEPTION_THROWN(j2.get<SmallStruct>(), nlohmann::detail::type_error);
    }
};

QTEST_MAIN(TestJsonify)
#include "tst_jsonify.moc"
