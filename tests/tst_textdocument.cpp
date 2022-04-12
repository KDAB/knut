#include "core/knutcore.h"
#include "core/textdocument.h"
#include "core/utils.h"

#include "common/test_utils.h"

#include <QDir>
#include <QFile>
#include <QTest>
#include <QTextStream>

static const QString LoremIpsumText(R"(
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
)");

class TestTextDocument : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void load()
    {
        Core::TextDocument document;
        document.load(Test::testDataPath() + "/textdocument/loremipsum_lf_utf8.txt");

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

        QTest::newRow("loremipsum_lf_utf8") << Test::testDataPath() + "/textdocument/loremipsum_lf_utf8.txt"
                                            << Core::TextDocument::LFLineEnding << false;
        QTest::newRow("loremipsum_crlf_ansi") << Test::testDataPath() + "/textdocument/loremipsum_crlf_ansi.txt"
                                              << Core::TextDocument::CRLFLineEnding << false;
        QTest::newRow("loremipsum_crlf_utf8") << Test::testDataPath() + "/textdocument/loremipsum_crlf_utf8.txt"
                                              << Core::TextDocument::CRLFLineEnding << false;
        QTest::newRow("loremipsum_crlf_utf8bom") << Test::testDataPath() + "/textdocument/loremipsum_crlf_utf8bom.txt"
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
        QVERIFY(Test::compareFiles(document.fileName(), Test::testDataPath() + "/textdocument/loremipsum_crlf_utf8.txt",
                                   false));
#else
        QVERIFY(Test::compareFiles(document.fileName(), Test::testDataPath() + "/textdocument/loremipsum_lf_utf8.txt",
                                   false));
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
        document.load(Test::testDataPath() + "/textdocument/loremipsum_lf_utf8.txt");

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
        document.load(Test::testDataPath() + "/textdocument/loremipsum_lf_utf8.txt");
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
        Test::FileTester file(Test::testDataPath() + "/textdocument/loremipsum_original.txt");
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
        document.load(Test::testDataPath() + "/textdocument/loremipsum_lf_utf8.txt");

        document.gotoLine(2);
        auto mark = document.createMark();
        document.gotoNextLine();
        document.selectToMark(mark);
        QCOMPARE(document.selectedText(), "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n");

        document.gotoEndOfDocument();
        document.gotoMark(mark);
        QCOMPARE(document.line(), 2);
    }

    void indent()
    {
        Test::FileTester file(Test::testDataPath() + "/textdocument/indent_original.txt");
        {
            Core::KnutCore core;
            Core::TextDocument document;
            document.load(file.fileName());

            document.gotoLine(4);
            document.indent();
            document.gotoLine(7, 4);
            document.removeIndent(2);
            document.gotoLine(10);
            document.selectNextLine();
            document.indent();
            document.gotoLine(16);
            document.selectNextLine();
            document.removeIndent();
            document.save();

            QVERIFY(file.compare());
        }
    }

    void findReplace()
    {
        Test::FileTester file(Test::testDataPath() + "/textdocument/findreplace_original.txt");
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

            document.save();
            QVERIFY(file.compare());
        }
    }
};

QTEST_MAIN(TestTextDocument)
#include "tst_textdocument.moc"
