#pragma once

#include <QObject>

namespace Core {

class File : public QObject
{
    Q_OBJECT

public:
    explicit File(QObject *parent = nullptr);
    ~File() override;

    Q_INVOKABLE bool copy(const QString &fileName, const QString &newName);
    Q_INVOKABLE bool exists(const QString &fileName);
    Q_INVOKABLE bool remove(const QString &fileName);
    Q_INVOKABLE bool rename(const QString &oldName, const QString &newName);

    Q_INVOKABLE bool touch(const QString &fileName);

    Q_INVOKABLE QString readAll(const QString &fileName);
};

} // namespace Core
