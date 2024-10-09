/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"
#include "treesitter/languages.h"
#include "treesitter/parser.h"
#include "treesitter/predicates.h"
#include "treesitter/query.h"
#include "treesitter/tree.h"

#include <QTest>

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

#define VERIFY_PREDICATE_ERROR(queryString)                                                                            \
    QVERIFY_THROWS_EXCEPTION(Error, treesitter::Query(tree_sitter_cpp(), queryString))

    void querySyntaxError()
    {
        using Error = treesitter::Query::Error;
        // Syntax Error - missing ")".
        VERIFY_PREDICATE_ERROR("(field_expression");
        // Invalid node type
        VERIFY_PREDICATE_ERROR("(field_expr)");
        // Invalid field
        VERIFY_PREDICATE_ERROR("(field_expression arg: (_))");
        // Capture Error
        VERIFY_PREDICATE_ERROR("(field_expression (#eq? @from @from))");
        // Structure Error
        VERIFY_PREDICATE_ERROR("(field_expression \"*\")");
        // Predicate errors
        // Non-existing predicate
        VERIFY_PREDICATE_ERROR("(#non_existing_predicate?)");
    }

    void simpleQuery()
    {
        auto source = readTestFile("/tst_treesitter/main.cpp");

        treesitter::Parser parser(tree_sitter_cpp());
        auto tree = parser.parseString(source);
        QVERIFY(tree.has_value());

        auto query = std::make_shared<treesitter::Query>(tree_sitter_cpp(), R"EOF(
        (field_expression
            argument: (_) @arg
            field: (_) @field
            (#eq? @arg "object")
            ) @from
                )EOF");

        auto captures = query->captures();
        QCOMPARE(captures.size(), 3);
        QCOMPARE(captures.at(0).name, "arg");
        QCOMPARE(captures.at(1).name, "field");
        QCOMPARE(captures.at(2).name, "from");

        auto patterns = query->patterns();
        QCOMPARE(patterns.size(), 1);

        const auto &pattern = patterns.first();
        QCOMPARE(pattern.predicates.size(), 1);
        QCOMPARE(pattern.predicates.first().name, "eq?");

        const auto &arguments = pattern.predicates.first().arguments;
        QCOMPARE(arguments.size(), 2);

        QVERIFY(std::holds_alternative<treesitter::Query::Capture>(arguments.at(0)));
        QCOMPARE(std::get<treesitter::Query::Capture>(arguments.at(0)).name, "arg");

        QVERIFY(std::holds_alternative<QString>(arguments.at(1)));
        QCOMPARE(std::get<QString>(arguments.at(1)), "object");

        treesitter::QueryCursor cursor;
        cursor.execute(query, tree->rootNode(), nullptr /*disable predicates*/);

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
        auto query = std::make_shared<treesitter::Query>(tree_sitter_cpp(), R"EOF(
        (field_expression
            argument: (_) @arg
            "->"
            field: (_) @field
            ) @from
                )EOF");

        treesitter::QueryCursor cursor;
        cursor.execute(query, tree->rootNode(), nullptr /*disable predicates*/);

        // The query should not match
        QVERIFY(!cursor.nextMatch().has_value());
    }

    void capture_quantifiers()
    {
        auto source = readTestFile("/tst_treesitter/main.cpp");

        treesitter::Parser parser(tree_sitter_cpp());
        auto tree = parser.parseString(source);

        auto query = std::make_shared<treesitter::Query>(tree_sitter_cpp(), R"EOF(
                (parameter_list
                    ["," (parameter_declaration) @arg]+)
        )EOF");

        treesitter::QueryCursor cursor;
        cursor.execute(query, tree->rootNode(), std::make_unique<treesitter::Predicates>(source));

        auto matches = cursor.allRemainingMatches();
        // 7 Matches, including declarations and function pointers
        QCOMPARE(matches.size(), 7);

        // TreeSitter will return multiple captures for the same capture identifier if
        // a quantifier is used.
        QCOMPARE(matches[0].captures().size(), 2);
        QCOMPARE(matches[1].captures().size(), 2);
        QCOMPARE(matches[2].captures().size(), 2); // function pointer list comes first
        QCOMPARE(matches[3].captures().size(), 6);
        QCOMPARE(matches[4].captures().size(), 2);
        QCOMPARE(matches[5].captures().size(), 6);
        QCOMPARE(matches[6].captures().size(), 2);
    }

    void eq_predicate_errors()
    {
        using Error = treesitter::Query::Error;
        // Too few arguments
        VERIFY_PREDICATE_ERROR(R"EOF(
        (#eq?)
        )EOF");
        // Still too few arguments
        VERIFY_PREDICATE_ERROR(R"EOF(
        (#eq? "test")
        )EOF");
    }

    // make sure we keep the tree alive, as otherwise the nodes will be dangling references
    std::tuple<QString, treesitter::Tree, treesitter::QueryCursor> runQuery(const QString &queryString)
    {
        auto source = readTestFile("/tst_treesitter/main.cpp");
        treesitter::Parser parser(tree_sitter_cpp());
        auto tree = parser.parseString(source);
        auto query = std::make_shared<treesitter::Query>(tree_sitter_cpp(), queryString);

        treesitter::QueryCursor cursor;
        cursor.execute(query, tree->rootNode(), std::make_unique<treesitter::Predicates>(source));

        return std::make_tuple(source, std::move(*tree), std::move(cursor));
    }

    void eq_predicate()
    {
        auto [source, tree, cursor] = runQuery(R"EOF(
            (function_definition
                (function_declarator
                    declarator: (_) @name
                    (#eq? "main" @name)
                    ))
        )EOF");

        auto firstMatch = cursor.nextMatch();

        QVERIFY(firstMatch.has_value());
        auto captures = firstMatch->capturesNamed("name");
        QCOMPARE(captures.size(), 1);
        QCOMPARE(captures.first().node.textIn(source), "main");

        QVERIFY(!cursor.nextMatch().has_value());
    }

    void match_predicate_errors()
    {
        using Error = treesitter::Query::Error;
        // Too few arguments
        VERIFY_PREDICATE_ERROR("(#match?)");

        // Non-regex argument
        VERIFY_PREDICATE_ERROR("((identifier) @ident (#match? @ident @ident)))");

        // Invalid regex
        VERIFY_PREDICATE_ERROR("((identifier) @ident (#match? \"tes[\" @ident))");

        // Non-capture argument
        VERIFY_PREDICATE_ERROR("(#match? \"test\" \"test\")");
    }

    void match_predicate()
    {
        auto [source, tree, cursor] = runQuery(R"EOF(
            (function_definition
                (function_declarator
                    declarator: (_) @name
                    (#match? "my(Other)?FreeFunction" @name)
                    ))
        )EOF");

        auto firstMatch = cursor.nextMatch();
        QVERIFY(firstMatch.has_value());
        auto firstCaptures = firstMatch->capturesNamed("name");
        QCOMPARE(firstCaptures.size(), 1);
        QCOMPARE(firstCaptures.first().node.textIn(source), "myFreeFunction");

        auto secondMatch = cursor.nextMatch();
        QVERIFY(secondMatch.has_value());
        auto secondCaptures = secondMatch->capturesNamed("name");
        QCOMPARE(secondCaptures.size(), 1);
        QCOMPARE(secondCaptures.first().node.textIn(source), "myOtherFreeFunction");

        QVERIFY(!cursor.nextMatch().has_value());
    }

    void in_message_map_predicate_errors()
    {
        using Error = treesitter::Query::Error;
        // Too few arguments
        VERIFY_PREDICATE_ERROR("(#in_message_map?)");

        // Non-capture argument
        VERIFY_PREDICATE_ERROR("(#in_message_map? \"xxxx\"))");
    }

    void in_message_map_predicate()
    {
        auto source = readTestFile("/tst_treesitter/mfc-TutorialDlg.cpp");
        treesitter::Parser parser(tree_sitter_cpp());
        auto tree = parser.parseString(source);
        QVERIFY(tree.has_value());

        auto query = std::make_shared<treesitter::Query>(tree_sitter_cpp(), R"EOF(
            (
            (call_expression
                (argument_list . (_) . (_) .) @args) @call
            (#in_message_map? @call @args))
        )EOF");

        treesitter::QueryCursor cursor;
        cursor.execute(query, tree->rootNode(), std::make_unique<treesitter::Predicates>(source));

        auto matches = cursor.allRemainingMatches();
        QCOMPARE(matches.size(), 3);
    }

    void eq_except_predicate_errors()
    {
        using Error = treesitter::Query::Error;
        // Too few arguments
        VERIFY_PREDICATE_ERROR("(_) @x (#eq_except? \"test\" @x)");
        // Non-capture argument
        VERIFY_PREDICATE_ERROR("(#eq_except? \"test\" \"\" \"\")");
        // Non-String argument
        VERIFY_PREDICATE_ERROR("(_) @x (#eq_except? @x \"\" \"\")");
        // Non-String type argument
        VERIFY_PREDICATE_ERROR("(_) @x (#eq_except? \"\" \"\" \"\" @x)");
    }

    void eq_except_predicate()
    {
        auto [source, tree, cursor] = runQuery(R"EOF(
            (
                (parameter_declaration) @param
                (#eq_except? "const std::string &" @param "identifier"))
        )EOF");

        auto match = cursor.nextMatch();
        QVERIFY(match.has_value());
        auto captures = match->capturesNamed("param");
        QCOMPARE(captures.size(), 1);
        QCOMPARE(captures.first().node.textIn(source), "const std::string &e_123");

        QVERIFY(!cursor.nextMatch().has_value());
    }

    void like_predicate_errors()
    {
        using Error = treesitter::Query::Error;
        // Too few arguments
        VERIFY_PREDICATE_ERROR("(#like?)");
        // Too few arguments
        VERIFY_PREDICATE_ERROR("(#like? \"test\")");
    }

    void like_predicate()
    {
        auto [source, tree, cursor] = runQuery(R"EOF(
            (function_definition
                declarator: (function_declarator
                    parameters: (parameter_list
                        (parameter_declaration) @param
                        (#like? "const std::string &" @param))))
        )EOF");

        auto match = cursor.nextMatch();
        QVERIFY(match.has_value());
        auto captures = match->capturesNamed("param");
        QCOMPARE(captures.size(), 1);
        QCOMPARE(captures.first().node.textIn(source), "const std::string&");

        QVERIFY(!cursor.nextMatch().has_value());
    }

    void like_except_predicate_errors()
    {
        using Error = treesitter::Query::Error;
        // Too few arguments
        VERIFY_PREDICATE_ERROR("(#like_except?)");
        // Only string arguments
        VERIFY_PREDICATE_ERROR("(#like_except? \"\" \"\" \"\")");
        // capture as expected argument
        VERIFY_PREDICATE_ERROR("(_) @x (#like_except? @x @x \"\")");
        // capture as filter argument
        VERIFY_PREDICATE_ERROR("(_) @x (#like_except? \"\" @x @x)");
    }

    void like_except_predicate()
    {
        auto [source, tree, cursor] = runQuery(R"EOF(
        (function_definition
            declarator: (function_declarator
                parameters: (parameter_list
                    (parameter_declaration) @param
                    (#like_except? "const std::string &" @param "identifier"))))
        )EOF");

        auto match = cursor.nextMatch();
        QVERIFY(match.has_value());
        auto captures = match->capturesNamed("param");
        QCOMPARE(captures.size(), 1);
        QCOMPARE(captures.first().node.textIn(source), "const std::string&");

        match = cursor.nextMatch();
        QVERIFY(match.has_value());
        captures = match->capturesNamed("param");
        QCOMPARE(captures.size(), 1);
        QCOMPARE(captures.first().node.textIn(source), "const std::string &e_123");

        QVERIFY(!cursor.nextMatch().has_value());
    }

    void exclude_predicate_errors()
    {
        using Error = treesitter::Query::Error;
        // Too few arguments
        VERIFY_PREDICATE_ERROR("(#exclude!)");
        // Only string arguments
        VERIFY_PREDICATE_ERROR("(#exclude! \"\" \"\")");
        // only capture arguments
        VERIFY_PREDICATE_ERROR("(_) @x (#exclude! @x @x)");
    }

    void exclude_predicate()
    {
        auto [source, tree, cursor] = runQuery(R"EOF(
        (call_expression
            function: (identifier) @name (#eq? @name freeFunction)
            arguments: (argument_list
                _* @argument)
            (#exclude! @argument comment "," "(" ")"))
        )EOF");

        auto match = cursor.nextMatch();
        QVERIFY(match.has_value());
        auto arguments = match->capturesNamed("argument");
        QCOMPARE(arguments.size(), 2);

        QCOMPARE(arguments[0].node.textIn(source), "1");
        QCOMPARE(arguments[1].node.textIn(source), "2");
    }

    // Comments are actually not documented on the tree-sitter.github.io website.
    // They can be used with ";"
    void comments()
    {
        auto source = readTestFile("/tst_treesitter/main.cpp");
        treesitter::Parser parser(tree_sitter_cpp());
        auto tree = parser.parseString(source);
        QVERIFY(tree.has_value());

        auto query = std::make_shared<treesitter::Query>(tree_sitter_cpp(), R"EOF(
        (call_expression
            ; some comment
            )
        )EOF");

        treesitter::QueryCursor cursor;
        cursor.execute(query, tree->rootNode(), std::make_unique<treesitter::Predicates>(source));

        auto matches = cursor.allRemainingMatches();
        QCOMPARE(matches.size(), 2);
    }

    void not_is_predicate()
    {
        auto source = readTestFile("/tst_treesitter/main.cpp");
        treesitter::Parser parser(tree_sitter_cpp());
        auto tree = parser.parseString(source);
        QVERIFY(tree.has_value());

        auto query = std::make_shared<treesitter::Query>(tree_sitter_cpp(), R"EOF(
            (function_definition
                type: (_) @type
                (#not_is? @type primitive_type))
        )EOF");

        treesitter::QueryCursor cursor;
        cursor.execute(query, tree->rootNode(), std::make_unique<treesitter::Predicates>(source));

        auto matches = cursor.allRemainingMatches();
        QCOMPARE(matches.size(), 1); // Only one function that returns a string, and not an int.
    }
};

QTEST_MAIN(TestTreeSitter)
#include "tst_treesitter.moc"
