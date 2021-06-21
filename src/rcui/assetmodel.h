#pragma once

#include "rcfile/data.h"

#include <QAbstractTableModel>
#include <QHash>

namespace RcUi {

class AssetModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns {
        ID,
        FileName,
    };

public:
    explicit AssetModel(const QVector<RcFile::Asset> &assets, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QVector<RcFile::Asset> m_assets;
};

} // namespace RcUi
