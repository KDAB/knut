/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "document.h"

#include <QObject>
#include <unordered_map>

namespace Lsp {
class Client;
}

namespace Core {

class Project : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString root READ root WRITE setRoot NOTIFY rootChanged)
    Q_PROPERTY(Core::Document *currentDocument READ currentDocument NOTIFY currentDocumentChanged)
    Q_PROPERTY(QList<Core::Document *> documents READ documents NOTIFY documentsChanged)

public:
    enum PathType {
        FullPath = 0,
        RelativeToRoot = 1,
    };
    Q_ENUM(PathType)

public:
    ~Project() override;

    static Project *instance();

    const QString &root() const;
    bool setRoot(const QString &newRoot);

    Core::Document *currentDocument() const;

    const QList<Document *> &documents() const;

    Q_INVOKABLE QStringList allFiles(Core::Project::PathType type = RelativeToRoot) const;
    Q_INVOKABLE QStringList allFilesWithExtension(const QString &extension,
                                                  Core::Project::PathType type = RelativeToRoot);
    Q_INVOKABLE QStringList allFilesWithExtensions(const QStringList &extensions,
                                                   Core::Project::PathType type = RelativeToRoot);
    Q_INVOKABLE QVariantList findInFiles(const QString &pattern) const;
    Q_INVOKABLE bool isFindInFilesAvailable() const;

public slots:
    Core::Document *get(const QString &fileName);
    Core::Document *open(const QString &fileName);
    void closeAll();
    void saveAllDocuments();
    Core::Document *openPrevious(int index = 1);

signals:
    void rootChanged();
    void currentDocumentChanged(Core::Document *document);
    void documentsChanged();

private:
    friend class KnutCore;
    explicit Project(QObject *parent = nullptr);

    Core::Document *getDocument(QString fileName, bool moveToBack = false);
    Lsp::Client *getClient(Document::Type type);

private:
    inline static Project *m_instance = nullptr;

    QString m_root;
    QList<Document *> m_documents;
    Core::Document *m_current = nullptr;
    std::unordered_map<Core::Document::Type, Lsp::Client *> m_lspClients;
};

} // namespace Core
