#pragma once

#include <QObject>

class TestUtil : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString testDataPath READ testDataPath CONSTANT)

public:
    explicit TestUtil(QObject *parent = nullptr);
    ~TestUtil() override;

    Q_INVOKABLE QString callerFile(int frameIndex = 0) const;
    Q_INVOKABLE int callerLine(int frameIndex = 0) const;

public slots:
    bool compareFiles(const QString &file, const QString &expected, bool eolLF = true);

    QString createTestProjectFrom(const QString &path);
    void removeTestProject(const QString &path);
    bool compareDirectories(const QString &current, const QString &expected);

    QString testDataPath() const;
};
