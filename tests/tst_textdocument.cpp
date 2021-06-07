#include "core/textdocument.h"
#include "core/utils.h"

#include "common/test_utils.h"

#include <QDir>
#include <QFile>
#include <QTest>
#include <QTextCodec>
#include <QTextStream>

static const QString loremIpsumText(R"(
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

        const auto text = document.text();
        QCOMPARE(text, loremIpsumText);
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

        document.setText(loremIpsumText);
        document.save();
        QVERIFY(Test::compareFiles(file, tempFile));

        // Cleanup
        QFile::remove(tempFile);
    }

    void save()
    {
        Core::TextDocument document;

        document.setText(loremIpsumText);
        QVERIFY(document.hasChanged());
        QCOMPARE(document.text(), loremIpsumText);

        const QString saveFileName = Core::Utils::mktemp("TestTextDocument");
        document.setFileName(saveFileName);
        document.save();
        QVERIFY(!document.hasChanged());
#if defined(Q_OS_WIN)
        QVERIFY(
            Test::compareFiles(document.fileName(), Test::testDataPath() + "/textdocument/loremipsum_crlf_utf8.txt"));
#else
        QVERIFY(Test::compareFiles(document.fileName(), Test::testDataPath() + "/textdocument/loremipsum_lf_utf8.txt"));
#endif
        document.setText("Not much to see");
        QVERIFY(document.hasChanged());
        const QString saveAsFileName = Core::Utils::mktemp("TestTextDocument");
        document.saveAs(saveAsFileName);
        QCOMPARE(document.fileName(), saveAsFileName);
        QVERIFY(!document.hasChanged());
        QVERIFY(!Test::compareFiles(saveFileName, saveAsFileName));

        // Cleanup
        QFile::remove(saveFileName);
        QFile::remove(saveAsFileName);
    }
};

QTEST_MAIN(TestTextDocument)
#include "tst_textdocument.moc"
