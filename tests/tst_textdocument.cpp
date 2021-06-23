#include "core/textdocument.h"
#include "core/utils.h"

#include "common/test_utils.h"

#include <QDir>
#include <QFile>
#include <QTest>
#include <QTextCodec>
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

        document.gotoPreviousCharacter(1);
        QCOMPARE(document.line(), 2);

        document.gotoDocumentStart();
        QCOMPARE(document.line(), 1);
        QCOMPARE(document.column(), 1);
        QCOMPARE(document.position(), 0);
        document.gotoDocumentEnd();
        QCOMPARE(document.line(), 21);
        QCOMPARE(document.position(), 888);

        document.gotoLine(8, 15);
        QCOMPARE(document.position(), 241);
        QCOMPARE(document.currentWord(), "sapien");
        QCOMPARE(document.currentLine(), "In venenatis sapien eu ornare sollicitudin.");

        document.gotoWordStart();
        QCOMPARE(document.position(), 240);
        document.gotoNextCharacter();
        document.gotoWordEnd();
        QCOMPARE(document.position(), 246);

        document.gotoNextWord();
        QCOMPARE(document.currentWord(), "eu");
        document.gotoNextWord(5);
        QCOMPARE(document.currentWord(), "Nulla");
        document.gotoNextCharacter();
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
        Core::TextDocument document;
        document.load(Test::testDataPath() + "/textdocument/loremipsum.txt");

        document.deleteRegion(598, 695);

        document.gotoLine(2);
        document.selectLineEnd();
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

        document.gotoLineEnd();
        document.deletePreviousCharacter(14);
        document.gotoNextLine(4);
        document.deleteNextCharacter(5);

        document.save();

        QVERIFY(file.compare());

        for (int i = 0; i < 9; ++i) // 9 editions done
            document.undo();

        QCOMPARE(document.text(), LoremIpsumText);
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

        document.gotoDocumentEnd();
        document.gotoMark(mark);
        QCOMPARE(document.line(), 2);
    }
};

QTEST_MAIN(TestTextDocument)
#include "tst_textdocument.moc"
