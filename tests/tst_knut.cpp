/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QTest>

#define KNUT_TEST(name)                                                                                                \
    void tst_##name()                                                                                                  \
    {                                                                                                                  \
        run_knut_test(#name);                                                                                          \
    }

class TestKnut : public QObject
{
    Q_OBJECT

private:
    void run_knut_test(const QString &name)
    {
        QFileInfo fi(Test::testDataPath() + QString("/tst_%1.qml").arg(name));
        QVERIFY(fi.exists());
        QStringList arguments {"--test", fi.absoluteFilePath()};
        QDir dir(Test::testDataPath() + "/tst_" + name);
        if (dir.exists())
            arguments.append(dir.absolutePath());
        const int failedTests = QProcess::execute(KNUT_BINARY_PATH, arguments);

        switch (failedTests) {
        case -2:
            QFAIL(QString("Failed to start Knut! (executable path: %1)").arg(KNUT_BINARY_PATH).toStdString().data());
            break;
        case -1:
            QFAIL("Knut crashed! (exit code: -1)");
            break;
        default:
            QCOMPARE(failedTests, 0);
            break;
        }
    }

private slots:
    KNUT_TEST(settings)
    KNUT_TEST(dir)
    KNUT_TEST(fileinfo)
    KNUT_TEST(utils)
    KNUT_TEST(rcdocument)
    KNUT_TEST(project)
};

QTEST_MAIN(TestKnut)
#include "tst_knut.moc"
