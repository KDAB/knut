#pragma once

#include <QObject>

namespace Core {

class TestUtil : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString testDataPath READ testDataPath CONSTANT)

public:
    explicit TestUtil(QObject *parent = nullptr);
    ~TestUtil() override;

    Q_INVOKABLE QString callerFile(int frameIndex = 0) const;
    Q_INVOKABLE int callerLine(int frameIndex = 0) const;

    static QString testDataPath();

public slots:
    static bool compareFiles(const QString &file, const QString &expected, bool eolLF = true);
    static QString createTestProjectFrom(const QString &path);
    static void removeTestProject(const QString &path);
    static bool compareDirectories(const QString &current, const QString &expected);
};

} // namespace Core
