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

public slots:
    static bool exists(const QString &file);

    static Core::QFileInfoValueType create(const QString &file);
};

} // namespace Core
