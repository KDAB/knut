#pragma once

#include "qfileinfovaluetype.h"

#include <QObject>

namespace Core {

class FileInfo : public QObject
{
    Q_OBJECT

public:
    explicit FileInfo(QObject *parent = nullptr);
    ~FileInfo() override;

    Q_INVOKABLE bool exists(const QString &file);

    Q_INVOKABLE Core::QFileInfoValueType create(const QString &file);
};

}
