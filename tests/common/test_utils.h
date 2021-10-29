#pragma once

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>

#include <spdlog/spdlog.h>

#include <memory>
#include <vector>

namespace Test {

inline QString testDataPath()
{
    QString path;
#if defined(TEST_DATA_PATH)
    path = TEST_DATA_PATH;
#endif
    if (path.isEmpty() || !QDir(path).exists()) {
        path = QCoreApplication::applicationDirPath() + "/test_data";
    }
    return path;
}

inline bool compareFiles(const QString &file, const QString &expected, bool eolLF = true)
{
    QFile file1(file);
    if (!file1.open(QIODevice::ReadOnly))
        return false;
    QFile file2(expected);
    if (!file2.open(QIODevice::ReadOnly))
        return false;

    auto data1 = file1.readAll();
    auto data2 = file2.readAll();
    if (eolLF) {
        data1.replace("\r\n", "\n");
        data2.replace("\r\n", "\n");
    }
    return data1 == data2;
}

/**
 * @brief The FileTester class to handle expected/original files
 * Create a temporary file based on an original one, and also compare to an expected one.
 * Delete the created file on destruction.
 */
class FileTester
{
public:
    FileTester(const QString &original)
        : m_original(original)
    {
        m_file = m_original;
        m_file.replace("_original", "");
        QFile::copy(m_original, m_file);
    }
    ~FileTester() { QFile::remove(m_file); }

    QString fileName() const { return m_file; }

    bool compare() const
    {
        QString expected = m_original;
        expected.replace("_original", "_expected");
        return compareFiles(m_file, expected);
    }

private:
    QString m_original;
    QString m_file;
};

class LogSilencer
{
public:
    inline static std::string Default = "";

    LogSilencer(const std::string &name = Default)
    {
        if (name.empty())
            m_logger = spdlog::default_logger();
        else
            m_logger = spdlog::get(name);
        if (m_logger) {
            m_level = m_logger->level();
            m_logger->set_level(spdlog::level::off);
        }
    }
    ~LogSilencer()
    {
        if (m_logger) {
            m_logger->set_level(m_level);
        }
    }

    LogSilencer(LogSilencer &&) noexcept = default;
    LogSilencer &operator=(LogSilencer &&) noexcept = default;

private:
    spdlog::level::level_enum m_level = spdlog::level::off;
    std::shared_ptr<spdlog::logger> m_logger;
};

class LogSilencers
{
public:
    LogSilencers(std::initializer_list<std::string> names)
    {
        m_logs.reserve(names.size());
        for (const auto &name : names)
            m_logs.emplace_back(name);
    }

private:
    std::vector<LogSilencer> m_logs;
};

constexpr inline bool noClangd()
{
#if defined(NO_CLANGD)
    return true;
#else
    return false;
#endif
}
}

// Check if clangd is available, needed for some tests
#define CHECK_CLANGD                                                                                                   \
    do {                                                                                                               \
        if constexpr (Test::noClangd())                                                                                \
            QSKIP("clangd is not available to run the tests");                                                         \
    } while (false)
