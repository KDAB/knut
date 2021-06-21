#pragma once

#include <QAbstractItemModel>
#include <QStringList>

namespace RcFile {
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
    explicit DataModel(const RcFile::Data &data, QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    const RcFile::Data &m_data;
};

} // namespace RcUi
