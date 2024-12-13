/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"
#include "core/knutcore.h"
#include "core/mark.h"
#include "core/rangemark.h"
#include "core/textdocument.h"
#include "core/utils.h"

#include <QDir>
#include <QFile>
#include <QTest>
#include <QTextStream>

static const char *LoremIpsumText = R"(
Lorem ipsum dolor sit amet, consectetur adipiscing elit.
Quisque convallis ipsum ac odio aliquet tincidunt.

Mauris ut magna vitae mauris fringilla condimentum.
Proin non mi placerat, ultricies diam sit amet, ultricies nisi.

In venenatis sapien eu ornare sollicitudin.
Nulla interdum tortor luctus sem viverra, vel auctor odio euismod.
Cras vel metus tempor, facilisis arcu non, semper odio.
Integer a erat quis neque dictum facilisis eu ac sapien.
Etiam imperdiet nunc nec nunc scelerisque, eget vehicula dolor fringilla.
Maecenas vehicula est nec diam volutpat, non porttitor lectus rhoncus.

Curabitur volutpat justo nec congue faucibus.
Nam pellentesque orci eu tortor lobortis suscipit.

Vestibulum posuere mauris in dolor volutpat, porttitor imperdiet ipsum imperdiet.
Sed sit amet augue rhoncus, convallis orci id, convallis dolor.
Duis vestibulum erat at placerat sollicitudin.
)";

class TestTextDocument : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void load()
    {
        Core::TextDocument document;
        document.load(Test::testDataPath() + "/tst_textdocument/loremipsum_lf_utf8.txt");

        QVERIFY(QFile::exists(document.fileName()));

        // Default values
        QVERIFY(document.fileName().endsWith("loremipsum_lf_utf8.txt"));
        QCOMPARE(document.lineCount(), 21);

        QCOMPARE(document.text(), LoremIpsumText);
    }

    void detectAndSaveCodec_data()
    {
        QTest::addColumn<QString>("file");
        QTest::addColumn<Core::TextDocument::LineEnding>("lineEnding");
        QTest::addColumn<bool>("bom");

        QTest::newRow("loremipsum_lf_utf8") << Test::testDataPath() + "/tst_textdocument/loremipsum_lf_utf8.txt"
                                            << Core::TextDocument::LFLineEnding << false;
        QTest::newRow("loremipsum_crlf_ansi") << Test::testDataPath() + "/tst_textdocument/loremipsum_crlf_ansi.txt"
                                              << Core::TextDocument::CRLFLineEnding << false;
        QTest::newRow("loremipsum_crlf_utf8") << Test::testDataPath() + "/tst_textdocument/loremipsum_crlf_utf8.txt"
                                              << Core::TextDocument::CRLFLineEnding << false;
        QTest::newRow("loremipsum_crlf_utf8bom")
            << Test::testDataPath() + "/tst_textdocument/loremipsum_crlf_utf8bom.txt"
            << Core::TextDocument::CRLFLineEnding << true;
    }

    void detectAndSaveCodec()
    {
        QFETCH(QString, file);
        QFETCH(Core::TextDocument::LineEnding, lineEnding);
        QFETCH(bool, bom);

        // Create a copy of the file
        const QString tempFile = QDir::tempPath() + "/testCodec.txt";
        QFile::remove(tempFile);
        QFile::copy(file, tempFile);

        Core::TextDocument document;
        document.load(tempFile);
        QCOMPARE(document.lineEnding(), lineEnding);
        QCOMPARE(document.hasUtf8Bom(), bom);

        document.setText(LoremIpsumText);
        document.save();
        QVERIFY(Test::compareFiles(file, tempFile, false));

        // Cleanup
        QFile::remove(tempFile);
    }

    void save()
    {
        Core::TextDocument document;

        document.setText(LoremIpsumText);
        QVERIFY(document.hasChanged());
        QCOMPARE(document.text(), LoremIpsumText);

        const QString saveFileName = Core::Utils::mktemp("TestTextDocument");
        document.saveAs(saveFileName);
        QVERIFY(!document.hasChanged());
#if defined(Q_OS_WIN)
        QVERIFY(Test::compareFiles(document.fileName(),
                                   Test::testDataPath() + "/tst_textdocument/loremipsum_crlf_utf8.txt", false));
#else
        QVERIFY(Test::compareFiles(document.fileName(),
                                   Test::testDataPath() + "/tst_textdocument/loremipsum_lf_utf8.txt", false));
#endif
        document.setText("Not much to see");
        QVERIFY(document.hasChanged());
        const QString saveAsFileName = Core::Utils::mktemp("TestTextDocument");
        document.saveAs(saveAsFileName);
        QCOMPARE(document.fileName(), saveAsFileName);
        QVERIFY(!document.hasChanged());
        QVERIFY(!Test::compareFiles(saveFileName, saveAsFileName, false));

        // Cleanup
        QFile::remove(saveFileName);
        QFile::remove(saveAsFileName);
    }

    void navigation()
    {
        Core::TextDocument document;
        document.load(Test::testDataPath() + "/tst_textdocument/loremipsum_lf_utf8.txt");

        QCOMPARE(document.line(), 1);
        QCOMPARE(document.column(), 1);

        document.gotoNextLine(2);
        QCOMPARE(document.line(), 3);

        document.gotoPreviousChar(1);
        QCOMPARE(document.line(), 2);

        document.gotoStartOfDocument();
        QCOMPARE(document.line(), 1);
        QCOMPARE(document.column(), 1);
        QCOMPARE(document.position(), 0);
        document.gotoEndOfDocument();
        QCOMPARE(document.line(), 21);
        QCOMPARE(document.position(), 888);

        document.gotoLine(8, 15);
        QCOMPARE(document.position(), 241);
        QCOMPARE(document.currentWord(), "sapien");
        QCOMPARE(document.currentLine(), "In venenatis sapien eu ornare sollicitudin.");

        document.gotoStartOfWord();
        QCOMPARE(document.position(), 240);
        document.gotoNextChar();
        document.gotoEndOfWord();
        QCOMPARE(document.position(), 246);

        document.gotoNextWord();
        QCOMPARE(document.currentWord(), "eu");
        document.gotoNextWord(5);
        QCOMPARE(document.currentWord(), "Nulla");
        document.gotoNextChar();
        // The next character move to the next line
        QCOMPARE(document.currentWord(), "Nulla");
    }

    void selection()
    {
        Core::TextDocument document;
        document.load(Test::testDataPath() + "/tst_textdocument/loremipsum_lf_utf8.txt");
        QVERIFY(document.selectedText().isEmpty());

        document.selectAll();
        QCOMPARE(document.selectedText(), LoremIpsumText);

        document.unselect();
        QVERIFY(document.selectedText().isEmpty());
        QCOMPARE(document.line(), 21); // cursor is at the end of the selection

        document.gotoLine(5, 10);
        document.selectNextLine();
        QCOMPARE(document.selectedText(), " magna vitae mauris fringilla condimentum.\nProin non");
        QCOMPARE(document.line(), 6);
        QCOMPARE(document.column(), 10);
    }

    void edition()
    {
        Test::FileTester file(Test::testDataPath() + "/tst_textdocument/edition/loremipsum.txt");
        {
            Core::TextDocument document;
            document.load(file.fileName());

            document.deleteRegion(598, 695);

            document.gotoLine(2);
            document.selectEndOfLine();
            document.copy();
            document.unselect();
            document.insert("\n");
            document.paste();

            document.gotoLine(6);
            document.insert("Hello World!\n");

            document.gotoNextLine();
            document.selectNextWord(2);
            document.deleteSelection();
            document.gotoNextWord(2);
            document.deleteEndOfLine();

            document.gotoLine(10, 4);
            document.replace(10, "homo-");

            document.gotoEndOfLine();
            document.deletePreviousCharacter(14);
            document.gotoNextLine(4);
            document.deleteNextCharacter(5);

            document.save();
            QVERIFY(file.compare());

            for (int i = 0; i < 9; ++i) // 9 editions done
                document.undo();

            QCOMPARE(document.text(), LoremIpsumText);
        }
    }

    void mark()
    {
        Core::TextDocument document;
        document.load(Test::testDataPath() + "/tst_textdocument/loremipsum_lf_utf8.txt");

        document.gotoLine(2);
        auto mark = document.createMark();
        QVERIFY(mark.isValid());
        QCOMPARE(mark.line(), 2);
        QCOMPARE(mark.column(), 1);

        document.gotoNextLine();
        document.selectToMark(mark);
        QCOMPARE(document.selectedText(), "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n");

        document.gotoEndOfDocument();
        document.gotoMark(mark);
        QCOMPARE(document.line(), 2);
    }

    void rangeMark()
    {
        Core::TextDocument document;
        document.load(Test::testDataPath() + "/tst_textdocument/loremipsum_lf_utf8.txt");

        document.gotoLine(2);
        document.selectEndOfLine();
        auto mark = document.createRangeMark();
        QCOMPARE(mark.text(), "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");

        document.gotoStartOfLine();
        document.selectEndOfWord();
        document.deleteSelection();
        QCOMPARE(mark.text(), " ipsum dolor sit amet, consectetur adipiscing elit.");

        // When replacing the text in the rangemark, the rangemark should span the new text afterwards.
        const auto newText = QString("Hello World");
        mark.replace(newText);
        document.selectRange(mark);
        QCOMPARE(document.selectedText(), newText);

        // Delete everything in the range mark.
        document.deleteSelection();
        // The range mark should still be valid, but span an empty range.
        QVERIFY(mark.isValid());
        QVERIFY(mark.text().isEmpty());
    }

    void rangeMark_textExcept()
    {
        Core::TextDocument document;
        document.load(Test::testDataPath() + "/tst_textdocument/loremipsum_lf_utf8.txt");

        document.gotoLine(2);
        document.selectEndOfLine();
        auto mark = document.createRangeMark();
        QCOMPARE(mark.text(), "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");

        document.gotoStartOfDocument();
        document.selectNextChar();
        document.selectEndOfWord();
        auto other = document.createRangeMark();
        QCOMPARE(other.text(), "\nLorem");
        QCOMPARE(mark.textExcept(other), " ipsum dolor sit amet, consectetur adipiscing elit.");
        QCOMPARE(other.textExcept(mark), "\n");

        document.gotoEndOfLine();
        document.gotoPreviousWord();
        document.gotoPreviousWord();
        document.gotoPreviousChar();
        document.selectNextWord();
        document.selectNextWord();
        other = document.createRangeMark();
        QCOMPARE(other.text(), " elit");
        QCOMPARE(mark.textExcept(other), "Lorem ipsum dolor sit amet, consectetur adipiscing.");
        QCOMPARE(other.textExcept(mark), "");

        document.gotoEndOfLine();
        document.gotoPreviousWord();
        document.selectNextLine();
        other = document.createRangeMark();
        QCOMPARE(other.text(), ".\nQuisque convallis ipsum ac odio aliquet tincidunt.");
        QCOMPARE(mark.textExcept(other), "Lorem ipsum dolor sit amet, consectetur adipiscing elit");
        QCOMPARE(other.textExcept(mark), "\nQuisque convallis ipsum ac odio aliquet tincidunt.");
    }

    void updateMark()
    {
        int mark = 10;

        Core::Mark::updateMark(mark, 12, 25, 30);
        // The mark doesn't change if something is edited after it.
        QVERIFY(mark == 10);

        Core::Mark::updateMark(mark, 10, 0, 2);
        // The mark is updated if something is inserted right before it.
        QVERIFY(mark == 12);

        Core::Mark::updateMark(mark, 10, 2, 0);
        // The mark is updated if something is removed right before it.
        QVERIFY(mark == 10);

        Core::Mark::updateMark(mark, 10, 2, 3);
        // The mark moves to the "from" position, if it is replaced with something else.
        QVERIFY(mark == 10);
    }

    void indent()
    {
        auto spaces = [](int count) {
            return QString(count, ' ');
        };
#define COMPARE_LINE_INDENT(LINE, TEXT, INDENT)                                                                        \
    QCOMPARE(document.indentTextAtLine(LINE), TEXT);                                                                   \
    QCOMPARE(document.indentationAtLine(LINE), INDENT);

#define COMPARE_CURRENT_INDENT(TEXT, INDENT)                                                                           \
    QCOMPARE(document.indentationAtPosition(document.position()), INDENT);                                             \
    QCOMPARE(document.indentationAtLine(-1), INDENT);                                                                  \
    QCOMPARE(document.indentTextAtPosition(document.position()), TEXT);                                                \
    QCOMPARE(document.indentTextAtLine(-1), TEXT);

        Test::FileTester file(Test::testDataPath() + "/tst_textdocument/indent/indent.txt");
        {
            Core::KnutCore core;
            Core::TextDocument document;
            document.load(file.fileName());

            COMPARE_LINE_INDENT(1, spaces(2), 0);
            COMPARE_LINE_INDENT(7, "  \t\t\t\t", 4);
            COMPARE_LINE_INDENT(29, spaces(4), 1);
            // If the line number is larger than the line count, an empty string is returned
            COMPARE_LINE_INDENT(50, "", 0);

            document.gotoLine(4);
            document.indent();
            COMPARE_CURRENT_INDENT(spaces(4), 1)

            // Test that we can correctly detect columns in mixed tabs and spaces.
            // In this test, the first column contains 2 spaces and a tab.
            // Because our tabsize is 4 spaces, the first 2 spaces and the tab combine into the first column.
            // When we remove 2 levels of indentation, that will result in 2 columns left (aka. 8 spaces).
            document.gotoLine(7, 4);
            COMPARE_CURRENT_INDENT("  \t\t\t\t", 4)
            document.indent(-2);
            COMPARE_CURRENT_INDENT(spaces(8), 2)

            document.gotoLine(10);
            document.selectNextLine();
            document.indent();

            document.gotoLine(16);
            document.selectNextLine();
            document.indent(-1);

            document.gotoLine(19);
            document.selectNextLine();
            document.setIndentation(1);

            document.indentLine(2, 25);
            document.indentLine(-1, 26);
            document.setIndentationAtLine(1, 28);

            document.save();

            QVERIFY(file.compare());
        }
#undef COMPARE_CURRENT_INDENT
#undef COMPARE_LINE_INDENT
    }

    void findReplace()
    {
        Test::FileTester file(Test::testDataPath() + "/tst_textdocument/findReplace/findreplace.txt");
        {
            Core::TextDocument document;
            document.load(file.fileName());

            document.gotoLine(4);
            QVERIFY(document.find("LOREM"));
            QCOMPARE(document.line(), 7);
            QCOMPARE(document.selectedText(), "Lorem");
            QVERIFY(!document.find("LOREM", Core::TextDocument::FindCaseSensitively));

            document.gotoLine(8);
            QVERIFY(document.find("Lor"));
            QCOMPARE(document.line(), 13);
            QCOMPARE(document.selectedText(), "Lor");
            QCOMPARE(document.currentWord(), "Lorem");
            QVERIFY(!document.find("Lor", Core::TextDocument::FindWholeWords));

            document.gotoLine(14);
            QVERIFY(document.find("m\\w*s",
                                  Core::TextDocument::FindCaseSensitively | Core::TextDocument::FindWholeWords
                                      | Core::TextDocument::FindRegexp));
            QCOMPARE(document.line(), 16);
            QCOMPARE(document.selectedText(), "mauris");
            document.insert("REPLACE TEXT");

            document.replaceAll("Lorem", "Merol");
            document.replaceAll("m\\w*s", "siruam",
                                Core::TextDocument::FindCaseSensitively | Core::TextDocument::FindWholeWords
                                    | Core::TextDocument::FindRegexp);
            document.replaceAll(", ([\\w ]*), ", "~~ \\1 ~~ ", Core::TextDocument::FindRegexp);

            const auto rangeMark = document.createRangeMark(233, 573); // line 7 to 14
            document.replaceAllInRange("Quisque convallis", "convallis Quisque", rangeMark);

            document.save();
            QVERIFY(file.compare());
        }
    }

    void findReplaceRegexForwards()
    {
        Test::FileTester file(Test::testDataPath() + "/tst_textdocument/findRegex/findregex.txt");
        {
            Core::TextDocument document;
            document.load(file.fileName());

            QString lorem_ipsum("(Lorem)\\s(ipsum)");
            QString ipsum_lorem("\\2 \\1");

            QVERIFY(document.replaceOne(lorem_ipsum, ipsum_lorem, Core::TextDocument::FindRegexp));
            QCOMPARE(document.replaceAllRegexp(lorem_ipsum, ipsum_lorem), 3);

            document.gotoLine(6);
            QVERIFY(document.findRegexp("Lor"));
            QCOMPARE(document.line(), 7);
            QCOMPARE(document.selectedText(), "Lor");
            QCOMPARE(document.currentWord(), "Lorem");
            QVERIFY(!document.findRegexp("Lor", Core::TextDocument::FindWholeWords));

            document.save();
            QVERIFY(file.compare());
        }
    }

    void findReplaceRegexBackwards()
    {
        Test::FileTester file(Test::testDataPath() + "/tst_textdocument/findRegex/findregex.txt");
        {
            Core::TextDocument document;
            document.load(file.fileName());

            QString lorem_ipsum("(Lorem)\\s(ipsum)");
            QString ipsum_lorem("\\2 \\1");

            QCOMPARE(document.replaceAllRegexp(lorem_ipsum, ipsum_lorem, Core::TextDocument::FindBackward), 4);

            document.gotoLine(8);
            QVERIFY(document.findRegexp("ips", Core::TextDocument::FindBackward));
            QCOMPARE(document.line(), 7);
            QCOMPARE(document.selectedText(), "ips");
            QCOMPARE(document.currentWord(), "ipsum");
            document.gotoEndOfDocument();
            QVERIFY(!document.findRegexp("ips", Core::TextDocument::FindWholeWords | Core::TextDocument::FindBackward));

            document.save();
            QVERIFY(file.compare());
        }
    }

    void findReplaceRegexInRange()
    {
        Test::FileTester file(Test::testDataPath() + "/tst_textdocument/findReplaceRegexInRange/loremipsum.txt");
        {
            Core::TextDocument document;
            document.load(file.fileName());

            document.gotoLine(4);
            document.gotoStartOfLine();
            document.selectNextLine();
            document.selectNextLine();

            auto range = document.createRangeMark();

            // remove the selection
            document.gotoLine(1);
            QCOMPARE(document.selectedText(), "");

            QCOMPARE(document.replaceAllRegexpInRange("Lorem", "Mauris", range), 1);
            document.save();
            QVERIFY(file.compare());
        }
    }

    // This test documents the desired behavior of any regexp matching in Knut.
    // All other regexp functions should exhibit similar behavior.
    //
    // Specifically this means:
    // - The regexp is matched against the entire document line-by-line (no multi-line support)
    // - Backwards matching is supported
    // - Case insensitivity is the default, case-sensitive is supported
    // - Matching is greedy by default, both forwards and backwards
    // - FindWholeWords is supported
    void match()
    {
        Core::TextDocument document;
        document.load(Test::testDataPath() + "/tst_textdocument/loremipsum_lf_utf8.txt");

        document.gotoStartOfDocument();

        // Match without a capture group doesn't match anything!
        QCOMPARE(document.match("Lorem"), "");
        QCOMPARE(document.selectedText(), "");
        QCOMPARE(document.position(), 0);

        // Match with a capture group matches the capture group and selects the match.
        auto lorqui = "(?<lor>Lorem)|(?<qui>Quisque)";
        QCOMPARE(document.match(lorqui), "lor");
        QCOMPARE(document.selectedText(), "Lorem");

        // Continued searching matches the next capture group.
        QCOMPARE(document.match(lorqui), "qui");
        QCOMPARE(document.selectedText(), "Quisque");

        // Matching is case-insensitive by default
        document.gotoStartOfDocument();
        auto lorquiLowercase = "(?<lor>lorem)|(?<qui>quisque)";
        QCOMPARE(document.match(lorquiLowercase), "lor");
        QCOMPARE(document.selectedText(), "Lorem");

        // Optional case-sensitivity
        QCOMPARE(document.match(lorquiLowercase, Core::TextDocument::FindCaseSensitively), "");
        QCOMPARE(document.selectedText(), "");
        QCOMPARE(document.match(lorquiLowercase, Core::TextDocument::PreserveCase), "");
        QCOMPARE(document.selectedText(), "");

        // The whole match is selected, even if the capture group doesn't span
        // the entire regexp.
        // Also: Matching is greedy in both forwards and backwards direction.
        document.gotoStartOfDocument();
        auto etit = R"EOF(\S*((?<et>et)|(?<it>it))\b)EOF";
        QCOMPARE(document.match(etit), "it");
        QCOMPARE(document.selectedText(), "sit");

        // Matching backwards works too.
        document.gotoEndOfDocument();
        QCOMPARE(document.match(etit, Core::TextDocument::FindBackward), "et");
        QCOMPARE(document.selectedText(), "amet");

        // Repeated as well.
        QCOMPARE(document.match(etit, Core::TextDocument::FindBackward), "it");
        QCOMPARE(document.selectedText(), "sit");

        // A failed match clears the selection but doesn't move the cursor!
        auto oldPosition = document.position();
        QCOMPARE(document.match("nothing"), "");
        QCOMPARE(document.selectedText(), "");
        QCOMPARE(document.position(), oldPosition);

        // Multiline matching is **NOT** supported
        document.gotoStartOfDocument();
        QCOMPARE(document.match("(?<loqui>Lorem.*Quisque)"), "");
        QCOMPARE(document.selectedText(), "");

        // FindWholeWords is supported
        document.gotoStartOfDocument();
        QCOMPARE(document.match("(?<lor>Lor)", Core::TextDocument::FindWholeWords), "");
        QCOMPARE(document.selectedText(), "");
    }

    // Test that findRegexp behaves like match
    //
    // Specifically this means:
    // - The regexp is matched against the entire document line-by-line (no multi-line support)
    // - Backwards matching is supported
    // - Case insensitivity is the default, case-sensitive is supported
    // - Matching is greedy by default, both forwards and backwards
    // - FindWholeWords is supported
    void findRegexp()
    {
        Core::TextDocument document;
        document.load(Test::testDataPath() + "/tst_textdocument/loremipsum_lf_utf8.txt");

        document.gotoStartOfDocument();

        // findRegexp selects the match
        auto lorqui = "(Lorem)|(Quisque)";
        QVERIFY(document.findRegexp(lorqui));
        QCOMPARE(document.selectedText(), "Lorem");

        // Continued searching matches the next capture group.
        QVERIFY(document.findRegexp(lorqui));
        QCOMPARE(document.selectedText(), "Quisque");

        // findRegexp is case-insensitive by default
        document.gotoStartOfDocument();
        auto lorquiLowercase = "(lorem)|(quisque)";
        QVERIFY(document.findRegexp(lorquiLowercase));
        QCOMPARE(document.selectedText(), "Lorem");

        // Optional case-sensitivity
        QVERIFY(!document.findRegexp(lorquiLowercase, Core::TextDocument::FindCaseSensitively));
        QCOMPARE(document.selectedText(), "");
        QVERIFY(!document.findRegexp(lorquiLowercase, Core::TextDocument::PreserveCase));
        QCOMPARE(document.selectedText(), "");

        // findRegexp is greedy in both forwards and backwards direction.
        document.gotoStartOfDocument();
        auto etit = R"EOF(\S*((et)|(it))\b)EOF";
        QVERIFY(document.findRegexp(etit));
        QCOMPARE(document.selectedText(), "sit");

        // Matching backwards works too.
        document.gotoEndOfDocument();
        QVERIFY(document.findRegexp(etit, Core::TextDocument::FindBackward));
        QCOMPARE(document.selectedText(), "amet");

        // Repeated as well.
        QVERIFY(document.findRegexp(etit, Core::TextDocument::FindBackward));
        QCOMPARE(document.selectedText(), "sit");

        // A failed findRegexp clears the selection but doesn't move the cursor!
        auto oldPosition = document.position();
        QVERIFY(!document.findRegexp("nothing"));
        QCOMPARE(document.selectedText(), "");
        QCOMPARE(document.position(), oldPosition);

        // Multiline regexp search is **NOT** supported
        document.gotoStartOfDocument();
        QEXPECT_FAIL("", "Multinline regexp search is NOT supported", Continue);
        QVERIFY(document.findRegexp("Lorem.*Quisque"));
        QCOMPARE(document.selectedText(), "");

        // FindWholeWords is supported
        document.gotoStartOfDocument();
        QVERIFY(!document.findRegexp("Lor", Core::TextDocument::FindWholeWords));
        QCOMPARE(document.selectedText(), "");
    }

    void findWholeWord()
    {
        Core::TextDocument document;

        // FindWholeWord should take `_` into account
        document.setText("IDOK_");
        QVERIFY(!document.find("IDOK", Core::TextDocument::FindWholeWords));

        // FindWholeWord should not take `:` into account
        document.setText("IDOK:");
        QVERIFY(document.find("IDOK", Core::TextDocument::FindWholeWords));
    }
};

QTEST_MAIN(TestTextDocument)
#include "tst_textdocument.moc"
