/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

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
    Q_PROPERTY(QString currentScriptPath READ currentScriptPath CONSTANT)
    Q_PROPERTY(QString homePath READ homePath CONSTANT)
    Q_PROPERTY(QString rootPath READ rootPath CONSTANT)
    Q_PROPERTY(QString tempPath READ tempPath CONSTANT)

public:
    explicit Dir(QString currentScriptPath, QObject *parent = nullptr);
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

    QChar separator() const;

    Q_INVOKABLE inline Core::QDirValueType current() const { return QDirValueType(currentPath()); }
    QString currentPath() const;

    Q_INVOKABLE inline Core::QDirValueType currentScript() const { return QDirValueType(currentScriptPath()); }
    QString currentScriptPath() const;

    Q_INVOKABLE inline Core::QDirValueType home() const { return QDirValueType(homePath()); }
    QString homePath() const;
    Q_INVOKABLE inline Core::QDirValueType root() const { return QDirValueType(rootPath()); }
    QString rootPath() const;
    Q_INVOKABLE inline Core::QDirValueType temp() const { return QDirValueType(tempPath()); }
    QString tempPath() const;

public slots:
    bool setCurrentPath(const QString &path);

    static QString toNativeSeparators(const QString &pathName);
    static QString fromNativeSeparators(const QString &pathName);

    static bool isRelativePath(const QString &path);
    static bool isAbsolutePath(const QString &path);

    static bool match(const QStringList &filters, const QString &fileName);
    static bool match(const QString &filter, const QString &fileName);

    static QString cleanPath(const QString &path);

    static Core::QDirValueType create(const QString &path);

signals:
    void currentPathChanged(const QString &path);

private:
    QString m_currentScriptPath;
};

} // namespace Core
