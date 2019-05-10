#include <QtTest>

#include "lexer.h"

#include <QByteArray>
#include <QFile>

class LexerTest : public QObject
{
    Q_OBJECT

private slots:
    void testType_data() {
        QTest::addColumn<QString>("input");
        QTest::addColumn<int>("type");
        QTest::newRow("string") << R"("Test")" << static_cast<int>(Parser::Token::String);
        QTest::newRow("comma") << "," << static_cast<int>(Parser::Token::Operator_Comma);
        QTest::newRow("operator_or") << "|" << static_cast<int>(Parser::Token::Operator_Or);
        QTest::newRow("directive") << "#directive" << static_cast<int>(Parser::Token::Directive);
        QTest::newRow("word") << "THIS_IS_A_WORD" << static_cast<int>(Parser::Token::Word);
    }
    void testType() {
        QFETCH(QString, input);
        QFETCH(int, type);

        Parser::Stream stream(input);
        Parser::Lexer lexer(stream);
        QCOMPARE(lexer.next()->type, type);
    }

    void testString_data() {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("token");
        QTest::newRow("sinple") << R"("Test")" << "Test";
        QTest::newRow("spaces") << R"("Test with spaces")" << "Test with spaces";
        QTest::newRow("escapes") << R"("Test with escapes "" \\ \n\t")" << "Test with escapes \" \\\\ \n\t";
        QTest::newRow("special chars") << R"("Test with special chars # , | /")" << "Test with special chars # , | /";
    }
    void testString() {
        QFETCH(QString, input);
        QFETCH(QString, token);

        Parser::Stream stream(input);
        Parser::Lexer lexer(stream);
        QCOMPARE(lexer.next()->toString(), token);
    }

    void testNext_data() {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("token");
        QTest::newRow("word,") << "WORD,1,2 3" << "WORD";
        QTest::newRow("word ") << "WORD 1,2 3" << "WORD";
        QTest::newRow("word|") << "WORD|1,2 3" << "WORD";
        QTest::newRow("word\\n") << "WORD\n1,2 3" << "WORD";
        QTest::newRow("word//") << "WORD//1,2 3" << "WORD";
    }
    void testNext() {
        QFETCH(QString, input);
        QFETCH(QString, token);

        Parser::Stream stream(input);
        Parser::Lexer lexer(stream);
        QCOMPARE(lexer.next()->toString(), token);
    }

    void testControl() {
      Parser::Stream stream(
          "    COMBOBOX         \"Text\",CLASS, -1, 05, 1, 234,STYLE_1 |"
          "                       STYLE_2 | STYLE_3");
      Parser::Lexer lexer(stream);
      QCOMPARE(lexer.peek()->type, Parser::Token::Keyword);
      QCOMPARE(lexer.next()->toKeyword(), Parser::Keywords::COMBOBOX);
      QCOMPARE(lexer.next()->toString(), "Text");
      QCOMPARE(lexer.next()->type, Parser::Token::Operator_Comma);
      QCOMPARE(lexer.next()->toKeyword(), Parser::Keywords::CLASS);
      QCOMPARE(lexer.next()->type, Parser::Token::Operator_Comma);
      QCOMPARE(lexer.next()->toInt(), -1);
      QCOMPARE(lexer.next()->type, Parser::Token::Operator_Comma);
      QCOMPARE(lexer.next()->toInt(), 5);
      QCOMPARE(lexer.next()->type, Parser::Token::Operator_Comma);
      QCOMPARE(lexer.next()->toInt(), 1);
      QCOMPARE(lexer.next()->type, Parser::Token::Operator_Comma);
      QCOMPARE(lexer.next()->toInt(), 234);
      QCOMPARE(lexer.next()->type, Parser::Token::Operator_Comma);
      QCOMPARE(lexer.next()->toString(), "STYLE_1");
      QCOMPARE(lexer.next()->type, Parser::Token::Operator_Or);
      QCOMPARE(lexer.next()->toString(), "STYLE_2");
      QCOMPARE(lexer.next()->type, Parser::Token::Operator_Or);
      QCOMPARE(lexer.next()->toString(), "STYLE_3");
    }

    void testDirective() {
        Parser::Stream stream(R"(#include "inc\\resource.h" //Comment)");
        Parser::Lexer lexer(stream);
        QCOMPARE(lexer.peek()->toString(), "include");
        QCOMPARE(lexer.next()->toString(), "include");
        QCOMPARE(lexer.next()->toString(), "inc\\\\resource.h");
        QCOMPARE(lexer.peek().has_value(), false);
        QCOMPARE(lexer.next().has_value(), false);
    }

    void testDlgInit() {
        Parser::Stream stream("    CONTROL, 0x403, 3, 0"
                           "    0x65f8, \"\\000\", ");
        Parser::Lexer lexer(stream);
        QCOMPARE(lexer.next()->toKeyword(), Parser::Keywords::CONTROL);
        QCOMPARE(lexer.next()->type, Parser::Token::Operator_Comma);
        QCOMPARE(lexer.next()->toString(), "0x403");
        QCOMPARE(lexer.next()->type, Parser::Token::Operator_Comma);
        QCOMPARE(lexer.next()->toInt(), 3);
        QCOMPARE(lexer.next()->type, Parser::Token::Operator_Comma);
        QCOMPARE(lexer.next()->toInt(), 0);
        QCOMPARE(lexer.next()->toString(), "0x65f8");
        QCOMPARE(lexer.next()->type, Parser::Token::Operator_Comma);
        QCOMPARE(lexer.next()->toString(), "\\000");
        QCOMPARE(lexer.next()->type, Parser::Token::Operator_Comma);
        QCOMPARE(lexer.next().has_value(), false);
    }

};


QTEST_APPLESS_MAIN(LexerTest)

#include "tst_lexertest.moc"
