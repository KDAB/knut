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

    void check() const { CHECK_FALSE(QProcess::execute(knutPath(), m_arguments)); }

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
    TEST_CASE("settings") { KnutRunner("tst_settings.qml").check(); }
}
