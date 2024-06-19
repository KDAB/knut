/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "utils/json.h"

#include <QDateTime>
#include <QFileSystemWatcher>
#include <QObject>

namespace Core {

class Document : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    Q_PROPERTY(bool exists READ exists NOTIFY existsChanged)
    Q_PROPERTY(Type type READ type CONSTANT)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(bool hasChanged READ hasChanged NOTIFY hasChangedChanged)

public:
    enum class Type {
        Cpp,
        Rc,
        Text,
        QtUi,
        Image,
        Slint,
        Qml,
        QtTs,
        Json,
    };
    Q_ENUM(Type)

public:
    explicit Document(Type type, QObject *parent = nullptr);

    const QString &fileName() const;
    void setFileName(const QString &newFileName);

    bool exists() const;

    Type type() const;

    const QString &errorString() const;

    bool hasChanged() const;

    bool hasChangedOnDisk() const;
    void reload();

public slots:
    bool load(const QString &fileName);
    bool save();
    bool saveAs(const QString &fileName);
    void close();

signals:
    void fileNameChanged();
    void existsChanged();
    void errorStringChanged();
    void hasChangedChanged();
    void fileUpdated();

protected:
    virtual bool doSave(const QString &fileName) = 0;
    virtual bool doLoad(const QString &fileName) = 0;

    virtual void didOpen() { }
    virtual void didClose() { }

    void setHasChanged(bool newHasChanged);
    void setErrorString(const QString &error);

private:
    enum ConflictResolution { KeepDiskChanges, OverwriteDiskChanges };
    ConflictResolution resolveConflictsOnSave() const;

    QString m_fileName;
    Type m_type;
    QString m_errorString;
    bool m_hasChanged = false;

    // Members used for refreshing file after external changes
    QDateTime m_lastModified;
};

NLOHMANN_JSON_SERIALIZE_ENUM(Document::Type,
                             {{Document::Type::Cpp, "cpp_type"},
                              {Document::Type::Text, "text_type"},
                              {Document::Type::Rc, "rc_type"},
                              {Document::Type::QtUi, "qtui_type"},
                              {Document::Type::Image, "image_type"},
                              {Document::Type::Slint, "slint_type"},
                              {Document::Type::QtTs, "qtts_type"},
                              {Document::Type::Qml, "qml_type"},
                              {Document::Type::Json, "json_type"}})

} // namespace Core

Q_DECLARE_METATYPE(Core::Document *)
