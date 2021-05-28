#include "core/knutmain.h"
#include "core/scriptmanager.h"

#include "common/test_utils.h"

#include <QDir>
#include <QFileInfo>

#include <QSignalSpy>
#include <QTest>

#define KNUT_TEST(name)                                                                                                \
    void tst_##name()                                                                                                  \
    {                                                                                                                  \
        Test::LogSilencer ls;                                                                                          \
        QFileInfo fi(Test::testDataPath() + "/tst_" #name ".qml");                                                     \
        QVERIFY(fi.exists());                                                                                          \
        QStringList arguments {"knut.exe", "-s", fi.absoluteFilePath()};                                               \
        QDir dir(Test::testDataPath() + "/" #name);                                                                    \
        if (dir.exists())                                                                                              \
            arguments.append({"-r", dir.absolutePath()});                                                              \
        Core::KnutMain main;                                                                                           \
        QSignalSpy finished(Core::ScriptManager::instance(), &Core::ScriptManager::scriptFinished);                    \
        main.process(arguments);                                                                                       \
        QVERIFY(finished.wait());                                                                                      \
        QCOMPARE(finished.takeFirst().at(0).toInt(), 0);                                                               \
    }

class TestKnut : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        Q_INIT_RESOURCE(core);
    }

    KNUT_TEST(settings)
    KNUT_TEST(dir)
    KNUT_TEST(fileinfo)
    KNUT_TEST(utils)
};

QTEST_MAIN(TestKnut)
#include "tst_knut.moc"
