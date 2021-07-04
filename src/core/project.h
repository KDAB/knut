#pragma once

#include "document.h"

#include <QObject>

#include <unordered_map>
#include <vector>

namespace Lsp {
class Client;
}

namespace Core {

class Project : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString root READ root WRITE setRoot NOTIFY rootChanged)
    Q_PROPERTY(Core::Document *currentDocument READ currentDocument NOTIFY currentDocumentChanged)

public:
    ~Project();

    static Project *instance();

    const QString &root() const;
    bool setRoot(const QString &newRoot);

    Core::Document *currentDocument() const;

public slots:
    QStringList allFiles() const;
    QStringList allFilesWithExtension(const QString &extension);

    Core::Document *open(QString fileName);

signals:
    void rootChanged();
    void currentDocumentChanged();

private:
    friend class KnutCore;
    explicit Project(QObject *parent = nullptr);

    Lsp::Client *getClient(Document::Type type);

private:
    inline static Project *m_instance = nullptr;

    QString m_root;
    std::vector<Document *> m_documents;
    Core::Document *m_current = nullptr;
    std::unordered_map<Core::Document::Type, Lsp::Client *> m_lspClients;
};

} // namespace Core
