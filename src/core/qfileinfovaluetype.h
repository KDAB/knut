#pragma once

#include "qdirvaluetype.h"

#include <QDateTime>
#include <QFileInfo>

namespace Core {

struct QFileInfoValueType
{
    Q_GADGET

    Q_PROPERTY(bool exists READ exists)
    Q_PROPERTY(QString filePath READ filePath)
    Q_PROPERTY(QString absoluteFilePath READ absoluteFilePath)
    Q_PROPERTY(QString canonicalFilePath READ canonicalFilePath)
    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString baseName READ baseName)
    Q_PROPERTY(QString completeBaseName READ completeBaseName)
    Q_PROPERTY(QString suffix READ suffix)
    Q_PROPERTY(QString bundleName READ bundleName)
    Q_PROPERTY(QString completeSuffix READ completeSuffix)
    Q_PROPERTY(QString path READ path)
    Q_PROPERTY(QString absolutePath READ absolutePath)
    Q_PROPERTY(QString canonicalPath READ canonicalPath)
    Q_PROPERTY(Core::QDirValueType dir READ dir)
    Q_PROPERTY(Core::QDirValueType absoluteDir READ absoluteDir)
    Q_PROPERTY(bool isReadable READ isReadable)
    Q_PROPERTY(bool isWritable READ isWritable)
    Q_PROPERTY(bool isExecutable READ isExecutable)
    Q_PROPERTY(bool isHidden READ isHidden)
    Q_PROPERTY(bool isRelative READ isRelative)
    Q_PROPERTY(bool isAbsolute READ isAbsolute)
    Q_PROPERTY(bool isFile READ isFile)
    Q_PROPERTY(bool isDir READ isDir)
    Q_PROPERTY(bool isSymLink READ isSymLink)
    Q_PROPERTY(bool isRoot READ isRoot)
    Q_PROPERTY(bool isBundle READ isBundle)
    Q_PROPERTY(QString symLinkTarget READ symLinkTarget)
    Q_PROPERTY(QString owner READ owner)
    Q_PROPERTY(uint ownerId READ ownerId)
    Q_PROPERTY(QString group READ group)
    Q_PROPERTY(uint groupId READ groupId)
    Q_PROPERTY(qint64 size READ size)
    Q_PROPERTY(QDateTime created READ created)
    Q_PROPERTY(QDateTime lastModified READ lastModified)
    Q_PROPERTY(QDateTime lastRead READ lastRead)

public:
    QFileInfoValueType();
    explicit QFileInfoValueType(const QString &file);

    Q_INVOKABLE QString toString() const;

    // Properties
    bool exists() const;

    QString filePath() const;
    QString absoluteFilePath() const;
    QString canonicalFilePath() const;
    QString fileName() const;
    QString baseName() const;
    QString completeBaseName() const;
    QString suffix() const;
    QString bundleName() const;
    QString completeSuffix() const;

    QString path() const;
    QString absolutePath() const;
    QString canonicalPath() const;
    QDirValueType dir() const;
    QDirValueType absoluteDir() const;

    bool isReadable() const;
    bool isWritable() const;
    bool isExecutable() const;
    bool isHidden() const;
    bool isNativePath() const;

    bool isRelative() const;
    bool isAbsolute() const;

    bool isFile() const;
    bool isDir() const;
    bool isSymLink() const;
    bool isRoot() const;
    bool isBundle() const;

    QString symLinkTarget() const;

    QString owner() const;
    uint ownerId() const;
    QString group() const;
    uint groupId() const;

    qint64 size() const;

    QDateTime created() const;
    QDateTime lastModified() const;
    QDateTime lastRead() const;

private:
    QFileInfo m_fileInfoValue;
};

} // namespace Core

Q_DECLARE_METATYPE(Core::QFileInfoValueType)
