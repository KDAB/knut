#include "assetmodel.h"

#include "global.h"

AssetModel::AssetModel(Data *data, const QHash<QString, Data::Asset> &assets, QObject *parent)
    : QAbstractTableModel(parent)
    , m_data(data)
    , m_assets(assets.values())
{
}

int AssetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_assets.size();
}

int AssetModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant AssetModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        const auto &asset = m_assets.at(index.row());
        switch (index.column()) {
        case ID: return asset.id;
        case FileName: return asset.fileName;
        }
    }

    if (role == Qt::ForegroundRole) {
        const auto &asset = m_assets.at(index.row());
        if (!asset.exist)
            return QVariant::fromValue(Qt::darkRed);
    }

    if (role == Knut::LineRole) {
        const auto &asset = m_assets.at(index.row());
        return asset.line;
    }

    return {};
}

QVariant AssetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QStringList headers = {"Id", "FileName"};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headers.value(section);
    }
    return {};
}
