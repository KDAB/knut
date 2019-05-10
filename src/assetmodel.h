#ifndef ASSETMODEL_H
#define ASSETMODEL_H

#include "data.h"

#include <QAbstractTableModel>
#include <QHash>
#include <QList>

class AssetModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns {
        ID,
        FileName,
    };

public:
    explicit AssetModel(Data *data, const QHash<QString, Data::Asset> &assets, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    Data *m_data;
    const QList<Data::Asset> m_assets;
};

#endif // ASSETMODEL_H
