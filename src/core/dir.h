#pragma once

#include "qdirvaluetype.h"

#include <QDir>
#include <QObject>

namespace Core {

class Dir : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QChar separator READ separator CONSTANT)
    Q_PROPERTY(QString currentPath READ currentPath WRITE setCurrentPath NOTIFY currentPathChanged)
    Q_PROPERTY(Core::QDirValueType current READ current NOTIFY currentPathChanged)
    Q_PROPERTY(QString currentScriptPath READ currentScriptPath CONSTANT)
    Q_PROPERTY(Core::QDirValueType currentScript READ currentScript CONSTANT)
    Q_PROPERTY(QString homePath READ homePath CONSTANT)
    Q_PROPERTY(Core::QDirValueType home READ home CONSTANT)
    Q_PROPERTY(QString rootPath READ rootPath CONSTANT)
    Q_PROPERTY(Core::QDirValueType root READ root CONSTANT)
    Q_PROPERTY(QString tempPath READ tempPath CONSTANT)
    Q_PROPERTY(Core::QDirValueType temp READ temp CONSTANT)

public:
    explicit Dir(QObject *parent = nullptr);
    ~Dir() override;

    enum Filter {
        Dirs = QDir::Dirs,
        Files = QDir::Files,
        Drives = QDir::Drives,
        NoSymLinks = QDir::NoSymLinks,
        AllEntries = QDir::AllEntries,
        TypeMask = QDir::TypeMask,
        Readable = QDir::Readable,
        Writable = QDir::Writable,
        Executable = QDir::Executable,
        PermissionMask = QDir::PermissionMask,
        Modified = QDir::Modified,
        Hidden = QDir::Hidden,
        System = QDir::System,
        AccessMask = QDir::AccessMask,
        AllDirs = QDir::AllDirs,
        CaseSensitive = QDir::CaseSensitive,
        NoDot = QDir::NoDot,
        NoDotDot = QDir::NoDotDot,
        NoDotAndDotDot = QDir::NoDotAndDotDot,
        NoFilter = QDir::NoFilter
    };
    Q_ENUM(Filter)
    Q_DECLARE_FLAGS(Filters, Filter)

    enum SortFlag {
        Name = QDir::Name,
        Time = QDir::Time,
        Size = QDir::Size,
        Unsorted = QDir::Unsorted,
        SortByMask = QDir::SortByMask,
        DirsFirst = QDir::DirsFirst,
        Reversed = QDir::Reversed,
        IgnoreCase = QDir::IgnoreCase,
        DirsLast = QDir::DirsLast,
        LocaleAware = QDir::LocaleAware,
        Type = QDir::Type,
        NoSort = QDir::NoSort
    };
    Q_ENUM(SortFlag)
    Q_DECLARE_FLAGS(SortFlags, SortFlag)

    Q_INVOKABLE QChar separator() const;

    bool setCurrentPath(const QString &path);
    inline Core::QDirValueType current() const { return QDirValueType(currentPath()); }
    QString currentPath() const;

    inline Core::QDirValueType currentScript() const { return QDirValueType(currentScriptPath()); }
    QString currentScriptPath() const;

    inline Core::QDirValueType home() const { return QDirValueType(homePath()); }
    QString homePath() const;
    inline Core::QDirValueType root() const { return QDirValueType(rootPath()); }
    QString rootPath() const;
    inline Core::QDirValueType temp() const { return QDirValueType(tempPath()); }
    QString tempPath() const;

    Q_INVOKABLE QString toNativeSeparators(const QString &pathName) const;
    Q_INVOKABLE QString fromNativeSeparators(const QString &pathName) const;

    Q_INVOKABLE bool isRelativePath(const QString &path) const;
    Q_INVOKABLE bool isAbsolutePath(const QString &path) const { return !isRelativePath(path); }

    Q_INVOKABLE bool match(const QStringList &filters, const QString &fileName) const;
    Q_INVOKABLE bool match(const QString &filter, const QString &fileName) const;

    Q_INVOKABLE QString cleanPath(const QString &path) const;

    Q_INVOKABLE Core::QDirValueType create(const QString &path) const;

signals:
    void currentPathChanged(const QString &path);
};

}
