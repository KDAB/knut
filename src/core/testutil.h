#pragma once

#include <QObject>

namespace Core {

class TestUtil : public QObject
{
    Q_OBJECT

public:
    explicit TestUtil(QObject *parent = nullptr);
    ~TestUtil() override;

public slots:
    QString callerFile(int frameIndex = 0) const;

    int callerLine(int frameIndex = 0) const;
};

}
