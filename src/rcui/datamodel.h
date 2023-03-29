#pragma once

#include <QAbstractItemModel>
#include <QStringList>

namespace RcCore {
struct RcFile;
}

namespace RcUi {

class DataModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Roles {
        TypeRole = Qt::UserRole,
        IndexRole,
        EmptyRole,
    };

public:
    explicit DataModel(const RcCore::RcFile &rcFile, QString language, QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setLanguage(const QString &language);

private:
    const RcCore::RcFile &m_rcFile;
    QString m_language;
};

} // namespace RcUi
