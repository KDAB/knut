/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"
#include "core/qttsdocument.h"

#include <QTest>

class TestQtTsDocument : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        Q_INIT_RESOURCE(core);
        Q_INIT_RESOURCE(rccore);
    }

    void load()
    {
        {
            Core::QtTsDocument document;
            document.load(Test::testDataPath() + QStringLiteral("/tst_qttsdocument/empty.ts"));
            QVERIFY(document.language().isEmpty());
        }
        {
            Core::QtTsDocument document;
            document.load(Test::testDataPath() + QStringLiteral("/tst_qttsdocument/language.ts"));
            QCOMPARE(document.language(), "EN_us");
        }
        {
            Core::QtTsDocument document;
            document.load(Test::testDataPath() + QStringLiteral("/tst_qttsdocument/language_context.ts"));
            QCOMPARE(document.language(), "FR_fr");
            QCOMPARE(document.messages().count(), 1);
            const auto message = document.messages().at(0);
            QCOMPARE(message->context(), "foo");
            QCOMPARE(message->source(), "text_translate");
            QCOMPARE(message->translation(), "text_french");
            QCOMPARE(message->fileName(), "bla.cpp");
        }
        {
            Core::QtTsDocument document;
            document.load(Test::testDataPath() + QStringLiteral("/tst_qttsdocument/language_several_messages.ts"));
            QCOMPARE(document.language(), "FR_fr");
            QCOMPARE(document.messages().count(), 2);
            const auto message = document.messages().at(0);
            QCOMPARE(message->context(), "foo");
        }
        {
            Core::QtTsDocument document;
            document.load(Test::testDataPath() + QStringLiteral("/tst_qttsdocument/language_several_contexts.ts"));
            QCOMPARE(document.language(), "FR_fr");
            QCOMPARE(document.messages().count(), 3);
            {
                const auto message = document.messages().at(0);
                QCOMPARE(message->context(), "context1");
                {
                    QCOMPARE(message->fileName(), "foo1.cpp");
                    QCOMPARE(message->source(), "text_translate_new1");
                    QCOMPARE(message->translation(), "text_french_new1");
                }
            }
            {
                const auto message = document.messages().at(1);
                QCOMPARE(message->context(), "foo");
                {
                    QCOMPARE(message->fileName(), "bla.cpp");
                    QCOMPARE(message->source(), "text_translate");
                    QCOMPARE(message->translation(), "text_french");
                }
            }
            {
                const auto message = document.messages().at(2);
                QCOMPARE(message->context(), "foo");
                {
                    QCOMPARE(message->fileName(), "foo.cpp");
                    QCOMPARE(message->source(), "text_translate_new");
                    QCOMPARE(message->translation(), "text_french_new");
                }
            }
        }
    }

    void add_messages()
    {
        Core::QtTsDocument document;
        document.load(Test::testDataPath() + QStringLiteral("/tst_qttsdocument/language_several_messages.ts"));
        QCOMPARE(document.language(), "FR_fr");
        QCOMPARE(document.messages().count(), 2);
        QCOMPARE(document.messages().at(0)->context(), "foo");

        // Add new context
        {
            document.addMessage("context_new", "new_loc", "original", "translated");
            QCOMPARE(document.messages().count(), 3);
            {
                const auto message = document.messages().at(2);
                QCOMPARE(message->fileName(), "new_loc");
                QCOMPARE(message->source(), "original");
                QCOMPARE(message->translation(), "translated");
                QCOMPARE(message->context(), "context_new");
            }
        }
        // Add new message in existing context
        {
            document.addMessage("foo", "new_loc", "original", "translated");
            QCOMPARE(document.messages().count(), 4);
            {
                const auto message = document.messages().at(3);
                QCOMPARE(message->fileName(), "new_loc");
                QCOMPARE(message->source(), "original");
                QCOMPARE(message->translation(), "translated");
                QCOMPARE(message->context(), "foo");
            }
        }
    }
};

QTEST_MAIN(TestQtTsDocument)
#include "tst_qttsdocument.moc"
