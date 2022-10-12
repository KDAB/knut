#include <QTest>

#include "common/test_utils.h"

#include "treesitter/languages.h"
#include "treesitter/parser.h"
#include "treesitter/query.h"
#include "treesitter/transformation.h"
#include "treesitter/tree.h"

class TestTreeSitter : public QObject
{
    Q_OBJECT

private:
    QString readTestFile(const QString &relativePath)
    {
        QFile mainfile(Test::testDataPath() + relativePath);
        if (!mainfile.open(QFile::ReadOnly | QFile::Text)) {
            spdlog::warn("Couldn't open file: {}", mainfile.fileName().toStdString());
            return "";
        }
        QTextStream in(&mainfile);
        return in.readAll();
    }

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void parsesMainFile()
    {
        auto source = readTestFile("/tst_treesitter/main.cpp");

        treesitter::Parser parser(tree_sitter_cpp());

        auto tree = parser.parseString(source);
        QVERIFY(tree.has_value());
        auto root = tree->rootNode();
        QVERIFY(!root.isNull());
        QVERIFY(!root.isMissing());
        QVERIFY(!root.hasError());

        QVERIFY(root.type() == "translation_unit");
        QCOMPARE(root.namedChildren().size(), 9);
    }

    void querySyntaxError()
    {
        using Error = treesitter::Query::Error;
        // Syntax Error - missing ")".
        QVERIFY_THROWS_EXCEPTION(Error, treesitter::Query(tree_sitter_cpp(), "(field_expression"));
        // Invalid node type
        QVERIFY_THROWS_EXCEPTION(Error, treesitter::Query(tree_sitter_cpp(), "(field_expr)"));
        // Invalid field
        QVERIFY_THROWS_EXCEPTION(Error, treesitter::Query(tree_sitter_cpp(), "(field_expression arg: (_))"));
        // Capture Error
        QVERIFY_THROWS_EXCEPTION(Error,
                                 treesitter::Query(tree_sitter_cpp(), "(field_expression (#match? @from @from))"));
        // Structure Error
        QVERIFY_THROWS_EXCEPTION(Error, treesitter::Query(tree_sitter_cpp(), "(field_expression \"*\")"));
    }

    void simpleQuery()
    {
        auto source = readTestFile("/tst_treesitter/main.cpp");

        treesitter::Parser parser(tree_sitter_cpp());
        auto tree = parser.parseString(source);
        QVERIFY(tree.has_value());

        treesitter::Query query(tree_sitter_cpp(), R"EOF(
        (field_expression
            argument: (_) @arg
            field: (_) @field
            (#testing @arg "hello world")) @from
                )EOF");

        auto captures = query.captures();
        QCOMPARE(captures.size(), 3);
        QCOMPARE(captures.at(0).name, "arg");
        QCOMPARE(captures.at(1).name, "field");
        QCOMPARE(captures.at(2).name, "from");

        auto patterns = query.patterns();
        QCOMPARE(patterns.size(), 1);

        const auto &pattern = patterns.first();
        QCOMPARE(pattern.predicates.size(), 1);
        QCOMPARE(pattern.predicates.first().name, "testing");

        const auto &arguments = pattern.predicates.first().arguments;
        QCOMPARE(arguments.size(), 2);

        QVERIFY(std::holds_alternative<treesitter::Query::Capture>(arguments.at(0)));
        QCOMPARE(std::get<treesitter::Query::Capture>(arguments.at(0)).name, "arg");

        QVERIFY(std::holds_alternative<QString>(arguments.at(1)));
        QCOMPARE(std::get<QString>(arguments.at(1)), "hello world");

        treesitter::QueryCursor cursor;
        cursor.execute(query, tree->rootNode());

        auto match = cursor.nextMatch();
        QVERIFY(match.has_value());
        QVERIFY(match->patternIndex() == 0);

        // Assure there is no second match
        auto nextMatch = cursor.nextMatch();
        QVERIFY(!nextMatch.has_value());
    }

    void failedQuery()
    {
        auto source = readTestFile("/tst_treesitter/main.cpp");

        treesitter::Parser parser(tree_sitter_cpp());
        auto tree = parser.parseString(source);
        QVERIFY(tree.has_value());

        // main.cpp only contains a field_expression with "." access, not "->" access.
        treesitter::Query query(tree_sitter_cpp(), R"EOF(
        (field_expression
            argument: (_) @arg
            "->"
            field: (_) @field
            ) @from
                )EOF");

        treesitter::QueryCursor cursor;
        cursor.execute(query, tree->rootNode());

        // The query should not match
        QVERIFY(!cursor.nextMatch().has_value());
    }

    void transformMemberAccess()
    {
        auto source = readTestFile("/tst_treesitter/main.cpp");

        treesitter::Parser parser(tree_sitter_cpp());
        auto tree = parser.parseString(source);
        QVERIFY(tree.has_value());

        treesitter::Query query(tree_sitter_cpp(), R"EOF(
        (field_expression
            argument: (_) @arg
            "."
            field: (_) @field
            ) @from
                )EOF");

        treesitter::Transformation transformation(source, std::move(parser), std::move(query), "@arg->@field");

        auto result = transformation.run();
        QCOMPARE(result, readTestFile("/tst_treesitter/main-arrow.cpp"));
    }

    void transformationErrors()
    {
        auto source = readTestFile("/tst_treesitter/main.cpp");

        {
            treesitter::Parser parser(tree_sitter_cpp());
            treesitter::Query query(tree_sitter_cpp(), R"EOF(
                (field_expression
                    argument: (_) @arg
                    "."
                    field: (_) @field
                    )
                )EOF");

            treesitter::Transformation missingFromTransformation(source, std::move(parser), std::move(query),
                                                                 "@arg->@field");
            QVERIFY_THROWS_EXCEPTION(treesitter::Transformation::Error, missingFromTransformation.run());
        }

        {
            treesitter::Parser parser(tree_sitter_cpp());
            treesitter::Query query(tree_sitter_cpp(), R"EOF(
                (field_expression
                    argument: (_) @arg
                    field: (_) @field
                    )
                )EOF");

            treesitter::Transformation recursiveTransformation(source, std::move(parser), std::move(query),
                                                               "@arg->@field");
            QVERIFY_THROWS_EXCEPTION(treesitter::Transformation::Error, recursiveTransformation.run());
        }
    }
};

QTEST_MAIN(TestTreeSitter)
#include "tst_treesitter.moc"
