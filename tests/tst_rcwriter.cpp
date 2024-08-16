/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"
#include "rccore/rcfile.h"

#include <QBuffer>
#include <QFile>
#include <QSet>
#include <QTest>
#include <QUiLoader>

using namespace RcCore;

class TestRcwriter : public QObject
{
    Q_OBJECT

private slots:
    void testQrc()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/2048Game/2048Game.rc");
        auto data = rcFile.data.value("LANG_ENGLISH;SUBLANG_ENGLISH_US");

        // Default settings test
        {
            QBuffer buffer;
            if (buffer.open(QIODevice::WriteOnly)) {
                auto assets = convertAssets(data, Asset::NoFlags);
                writeAssetsToQrc(assets, &buffer, Test::testDataPath() + "/rcfiles/2048Game/2048Game.qrc");
                buffer.close();
            }
            if (buffer.open(QIODevice::ReadOnly)) {
                QFile file(Test::testDataPath() + "/tst_rcwriter/qrc_expected_default.qrc");
                file.open(QIODevice::ReadOnly);
                QCOMPARE(buffer.readAll(), file.readAll());
            }
        }

        // Don't add if asset does not exist, don't use aliases
        {
            QFile buffer;
            if (buffer.open(QIODevice::WriteOnly)) {
                auto assets = convertAssets(data);
                writeAssetsToQrc(assets, &buffer, Test::testDataPath() + "/rcfiles/2048Game/2048Game.qrc");
                buffer.close();
            }
            if (buffer.open(QIODevice::ReadOnly)) {
                QFile file(Test::testDataPath() + "/tst_rcwriter/qrc_expected_noalias_onlyexist.qrc");
                file.open(QIODevice::ReadOnly);
                QCOMPARE(buffer.readAll(), file.readAll());
            }
        }
    }

    void testConvertDialog()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/2048Game/2048Game.rc");
        auto usData = rcFile.data.value("LANG_ENGLISH;SUBLANG_ENGLISH_US");
        auto result = convertDialog(usData, usData.dialogs.first(), RcCore::Widget::AllFlags);

        QCOMPARE(result.id, "IDD_ABOUTBOX");
        QCOMPARE(result.geometry, QRect(0, 0, 255, 103));
        QCOMPARE(result.className, "QDialog");
        QCOMPARE(result.properties["windowTitle"].toString(), "About 2048Game");

        auto ukData = rcFile.data.value("LANG_UKRAINIAN;SUBLANG_DEFAULT");
        result = convertDialog(ukData, ukData.dialogs.first(), RcCore::Widget::AllFlags);
        QCOMPARE(result.children.size(), 6);
        auto item = result.children.at(2);
        QCOMPARE(item.className, "QPushButton");
        QCOMPARE(item.properties.value("text").toString(), "OK");
        QCOMPARE(item.geometry, QRect(96, 108, 75, 24));
        item = result.children.last();
        QCOMPARE(item.className, "QComboBox");
        QStringList values = {"3", "4", "5", "6"};
        QCOMPARE(item.properties.value("text").toStringList(), values);
    }
    void testStaticControlAlignment()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/alignment/alignment_test.rc");
        QString expectedLanguage = "LANG_ENGLISH;SUBLANG_ENGLISH_US";
        QVERIFY2(rcFile.data.contains(expectedLanguage),
                 qPrintable(QString("RC file does not contain the expected language: %1").arg(expectedLanguage)));
        auto data = rcFile.data.value(expectedLanguage);
        QVERIFY(!data.dialogs.isEmpty());
        auto result = convertDialog(data, data.dialogs.first(), RcCore::Widget::AllFlags);

        QMap<QString, QPair<int, bool>> expectedProperties;
        expectedProperties["IDC_STATIC_LEFT"] = {Qt::AlignLeft | Qt::AlignVCenter, true};
        expectedProperties["IDC_STATIC_RIGHT"] = {Qt::AlignRight | Qt::AlignVCenter, true};
        expectedProperties["IDC_STATIC_CENTER"] = {Qt::AlignHCenter | Qt::AlignVCenter, true};
        expectedProperties["IDC_STATIC_LEFT_NOWRAP"] = {Qt::AlignLeft | Qt::AlignVCenter, false};
        expectedProperties["IDC_STATIC_CENTER_TEXT"] = {Qt::AlignHCenter | Qt::AlignVCenter, true};
        expectedProperties["IDC_STATIC_RIGHT_TEXT"] = {Qt::AlignRight | Qt::AlignVCenter, true};
        expectedProperties["IDC_STATIC_WORDELLIPSIS"] = {Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, true};
        expectedProperties["IDC_STATIC_CENTER_IMAGE"] = {Qt::AlignCenter, true};
        expectedProperties["IDC_STATIC"] = {Qt::AlignLeft | Qt::AlignVCenter, true};

        QSet<QString> foundIds;
        QSet<QString> expectedIds = QSet<QString>(expectedProperties.keyBegin(), expectedProperties.keyEnd());

        for (const auto &child : result.children) {
            if (child.className == "QLabel" && expectedProperties.contains(child.id)) {
                foundIds.insert(child.id);
                QVERIFY2(child.properties.contains("alignment"),
                         qPrintable(QString("Alignment missing for %1").arg(child.id)));
                QVERIFY2(child.properties.contains("wordWrap"),
                         qPrintable(QString("WordWrap missing for %1").arg(child.id)));

                QCOMPARE(child.properties["alignment"].toInt(), expectedProperties[child.id].first);
                QCOMPARE(child.properties["wordWrap"].toBool(), expectedProperties[child.id].second);
            }
        }

        // Ensure we've checked all expected alignments
        QCOMPARE(foundIds, expectedIds);

        // Test writing to UI
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        writeDialogToUi(result, &buffer, QString());
        buffer.close();

        // Now read the buffer and check if alignments are correctly written
        buffer.open(QIODevice::ReadOnly);
        QString uiContent = QString::fromUtf8(buffer.readAll());

        for (const auto &id : expectedIds) {
            QStringList alignmentFlags;
            int alignment = expectedProperties[id].first;
            if (alignment & Qt::AlignLeft)
                alignmentFlags << "Qt::AlignLeft";
            if (alignment & Qt::AlignRight)
                alignmentFlags << "Qt::AlignRight";
            if (alignment & Qt::AlignHCenter)
                alignmentFlags << "Qt::AlignHCenter";
            if (alignment & Qt::AlignJustify)
                alignmentFlags << "Qt::AlignJustify";
            if (alignment & Qt::AlignTop)
                alignmentFlags << "Qt::AlignTop";
            if (alignment & Qt::AlignBottom)
                alignmentFlags << "Qt::AlignBottom";
            if (alignment & Qt::AlignVCenter)
                alignmentFlags << "Qt::AlignVCenter";
            if (alignment & Qt::TextWordWrap)
                alignmentFlags << "Qt::TextWordWrap";

            QString alignmentString = alignmentFlags.join(" | ");

            // Find the actual alignment property in the UI content
            QRegularExpression re(QString("<widget class=\"QLabel\" name=\"%1\">[\\s\\S]*?<property "
                                          "name=\"alignment\">[\\s\\S]*?<set>(.*?)</set>[\\s\\S]*?</property>")
                                      .arg(id));
            QRegularExpressionMatch match = re.match(uiContent);
            if (match.hasMatch()) {
                QString actualAlignment = match.captured(1).trimmed();
                // Compare the actual and expected alignments
                QSet<QString> expectedFlags;
                for (const QString &flag : alignmentFlags) {
                    expectedFlags.insert(flag);
                }
                QSet<QString> actualFlags;
                QStringList actualFlagList = actualAlignment.split(" | ");
                for (const QString &flag : actualFlagList) {
                    actualFlags.insert(flag);
                }

                QCOMPARE_EQ(actualFlags, expectedFlags);
            } else {
                qDebug() << "Actual: Not found";
                QFAIL(qPrintable(QString("Alignment property not found for %1").arg(id)));
            }

            // Check wordWrap property
            bool expectedWordWrap = expectedProperties[id].second;
            QString expectedWordWrapString = expectedWordWrap ? "true" : "false";
            QRegularExpression wordWrapRe(QString("<widget class=\"QLabel\" name=\"%1\">[\\s\\S]*?<property "
                                                  "name=\"wordWrap\">[\\s\\S]*?<bool>(.*?)</bool>[\\s\\S]*?</property>")
                                              .arg(id));
            QRegularExpressionMatch wordWrapMatch = wordWrapRe.match(uiContent);
            if (wordWrapMatch.hasMatch()) {
                QString actualWordWrap = wordWrapMatch.captured(1).trimmed();
                QCOMPARE_EQ(actualWordWrap, expectedWordWrapString);
            } else {
                qDebug() << "WordWrap property not found";
                QFAIL(qPrintable(QString("WordWrap property not found for %1").arg(id)));
            }
        }
    }
    void testWriteDialog()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/cryEdit/CryEdit.rc");
        auto data = rcFile.data.value("LANG_ENGLISH;SUBLANG_ENGLISH_US");

        QUiLoader loader;
        QSet<QString> dialogIds = {
            QLatin1String("IDD_ABCCOMPILE"),           QLatin1String("IDD_CHARACTER_EDITOR_AUTO_EVENTS"),
            QLatin1String("IDD_CHARPANEL_ANIMATION"),  QLatin1String("IDD_LIGHTING"),
            QLatin1String("IDD_PANEL_DISPLAY_STEREO"), QLatin1String("IDD_PANEL_TERRAIN_MODIFY"),
        };

        for (int index = 0; index < data.dialogs.size(); ++index) {
            Widget dialog = convertDialog(data, data.dialogs.value(index), RcCore::Widget::AllFlags);

            QBuffer buffer;
            if (buffer.open(QIODevice::WriteOnly)) {
                writeDialogToUi(dialog, &buffer);
                buffer.close();
            }
            if (buffer.open(QIODevice::ReadOnly)) {
                if (dialogIds.contains(dialog.id)) {
                    QFile file(Test::testDataPath() + QStringLiteral("/tst_rcwriter/%1.ui").arg(dialog.id));
                    // Regenerate the expected files
                    {
                        file.open(QIODevice::WriteOnly);
                        file.write(buffer.data());
                        file.close();
                    }
                    file.open(QIODevice::ReadOnly);
                    QCOMPARE(buffer.readAll(), file.readAll());
                } else {
                    QWidget *widget = loader.load(&buffer);
                    QVERIFY2(loader.errorString().isEmpty(), dialog.id.toLatin1());
                    widget->deleteLater();
                }
            }
        }
    }

    void testConvertAction()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/2048Game/2048Game.rc");
        auto data = rcFile.data.value("LANG_ENGLISH;SUBLANG_ENGLISH_US");

        // MainFrame menu and shortcuts and toolbar
        auto result = convertActions(data);

        QCOMPARE(result.size(), 51);
        auto action = result.first();
        QCOMPARE(action.id, "ID_FILE_NEW");
        QCOMPARE(action.title, "&New");
        QCOMPARE(action.toolTip, "New");
        QCOMPARE(action.statusTip, "Create a new document");
        QCOMPARE(action.shortcuts.size(), 1);
        QCOMPARE(action.shortcuts.first().event, "Ctrl+N");
        QCOMPARE(action.shortcuts.last().event, "Ctrl+N");
        QVERIFY(action.iconPath.endsWith("res/Toolbar_0.png"));

        action = result.value(12);
        QCOMPARE(action.id, "ID_EDIT_PASTE");
        QCOMPARE(action.title, "&Paste");
        QCOMPARE(action.shortcuts.size(), 2);
        QCOMPARE(action.shortcuts.first().event, "Ctrl+V");
        QCOMPARE(action.shortcuts.last().event, "Shift+Ins");
        QVERIFY(action.iconPath.endsWith("res/Toolbar_5.png"));

        action = result.at(40);
        QCOMPARE(action.id, "ID_PREV_PANE");
        QCOMPARE(action.toolTip, "Previous Pane");
        QCOMPARE(action.statusTip, "Switch back to the previous window pane");
        QCOMPARE(action.shortcuts.size(), 1);
        QCOMPARE(action.shortcuts.first().event, "Shift+F6");
    }
};

QTEST_MAIN(TestRcwriter)

#include "tst_rcwriter.moc"
