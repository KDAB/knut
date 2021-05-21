#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

class KnutRunner
{
public:
    KnutRunner(const QString &scriptName)
    {
        QFileInfo fi(testDataPath() + '/' + scriptName);
        REQUIRE(fi.exists());
        m_arguments.append({"-s", fi.absoluteFilePath()});
    }

    KnutRunner &root(const QString &rootDir)
    {
        QDir dir(testDataPath() + '/' + rootDir);
        REQUIRE(dir.exists());
        m_arguments.append({"-r", dir.absolutePath()});
        return *this;
    }

    void check() const
    {
        // 0 Means a success
        // >0: number of failed tests
        // <0: problem loading the script
        CHECK_EQ(QProcess::execute(knutPath(), m_arguments), 0);
    }

protected:
    QString knutPath() const
    {
        QString path;
#if defined(KNUT_EXE)
        path = KNUT_EXE;
#endif
        if (path.isEmpty() || !QFile::exists(path)) {
#ifdef Q_OS_WINDOWS
            return "knut.exe";
#else
            return "knut";
#endif
        }
        return path;
    }

    QString testDataPath() const
    {
        QString path;
#if defined(TEST_DATA_PATH)
        path = TEST_DATA_PATH;
#endif
        if (path.isEmpty() || !QDir(path).exists())
            return "test_data";
        return path;
    }

private:
    QStringList m_arguments;
};

TEST_SUITE("knut")
{
    TEST_CASE("settings") { KnutRunner("tst_settings.qml").root("settings").check(); }
}
