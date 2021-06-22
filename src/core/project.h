#pragma once

#include <QAbstractItemModel>
#include <QObject>

namespace Core {

class Document;

class Project : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString root READ root WRITE setRoot NOTIFY rootChanged)
    Q_PROPERTY(QAbstractItemModel *fileModel READ fileModel NOTIFY rootChanged)

public:
    ~Project();

    static Project *instance();

    const QString &root() const;
    bool setRoot(const QString &newRoot);

    QAbstractItemModel *fileModel() const;

public slots:
    QStringList allFiles() const;
    QStringList allFilesWithExtension(const QString &extension);

    Core::Document *open(QString fileName);

signals:
    void rootChanged();

private:
    friend class KnutMain;
    explicit Project(QObject *parent = nullptr);

private:
    inline static Project *m_instance = nullptr;

    QString m_root;
    std::vector<Document *> m_documents;
};

} // namespace Core
