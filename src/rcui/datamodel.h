#pragma once

#include <QAbstractItemModel>
#include <QStringList>

namespace RcCore {
struct Data;
}

namespace RcUi {

class DataModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Roles {
        TypeRole = Qt::UserRole,
        IndexRole,
    };

public:
    explicit DataModel(const RcCore::Data &data, QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    const RcCore::Data &m_data;
};

} // namespace RcUi
