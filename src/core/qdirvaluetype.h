#pragma once

#include <QDir>

namespace Core {

struct QDirValueType
{
    Q_GADGET

    Q_PROPERTY(QString path READ path WRITE setPath)
    Q_PROPERTY(QString absolutePath READ absolutePath)
    Q_PROPERTY(QString canonicalPath READ canonicalPath)
    Q_PROPERTY(QString dirName READ dirName)
    Q_PROPERTY(uint count READ count)
    Q_PROPERTY(bool isReadable READ isReadable)
    Q_PROPERTY(bool exists READ exists)
    Q_PROPERTY(bool isRoot READ isRoot)
    Q_PROPERTY(bool isRelative READ isRelative)
    Q_PROPERTY(bool isAbsolute READ isAbsolute)

public:
    QDirValueType(const QString &path = QString());
    QDirValueType(const QDir &dir);

    Q_INVOKABLE QString toString() const;

    // Properties
    QString path() const;
    void setPath(const QString &path);
    QString absolutePath() const;
    QString canonicalPath() const;
    QString dirName() const;
    qsizetype count() const;
    bool isReadable() const;
    bool exists() const;
    bool isRoot() const;
    bool isRelative() const;
    bool isAbsolute() const;

    // QDir methods
    Q_INVOKABLE bool cd(const QString &dirName);
    Q_INVOKABLE bool cdUp();

    Q_INVOKABLE QString at(int pos) const;

    Q_INVOKABLE QStringList entryList(int filters = QDir::NoFilter, int sort = QDir::NoSort) const;
    Q_INVOKABLE QStringList entryList(const QString &nameFilter, int filters = QDir::NoFilter,
                                      int sort = QDir::NoSort) const;
    Q_INVOKABLE QStringList entryList(const QStringList &nameFilters, int filters = QDir::NoFilter,
                                      int sort = QDir::NoSort) const;

    Q_INVOKABLE bool mkdir(const QString &dirName) const;
    Q_INVOKABLE bool rmdir(const QString &dirName) const;
    Q_INVOKABLE bool mkpath(const QString &dirPath) const;
    Q_INVOKABLE bool rmpath(const QString &dirPath) const;

    Q_INVOKABLE bool removeRecursively();

    Q_INVOKABLE bool makeAbsolute();

    Q_INVOKABLE bool remove(const QString &fileName);
    Q_INVOKABLE bool rename(const QString &oldName, const QString &newName);
    Q_INVOKABLE bool fileExists(const QString &name) const;

private:
    QDir m_dirValue;
};

} // namespace Core

Q_DECLARE_METATYPE(Core::QDirValueType)
