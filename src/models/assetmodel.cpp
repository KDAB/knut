#include "assetmodel.h"

#include "global.h"

#include <QColor>

AssetModel::AssetModel(const QHash<QString, Data::Asset> &assets, QObject *parent)
    : QAbstractTableModel(parent)
    , m_assets(assets.values())
{
    Knut::sort(m_assets, [](const auto &left, const auto &right) { return left.id < right.id; });
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
        case ID:
            return asset.id;
        case FileName:
            return asset.fileName;
        }
    }

    if (role == Qt::ForegroundRole) {
        const auto &asset = m_assets.at(index.row());
        if (!asset.exist)
            return QVariant::fromValue(QColor(Qt::red));
    }

    if (role == Knut::LineRole) {
        const auto &asset = m_assets.at(index.row());
        return asset.line;
    }

    return {};
}

QVariant AssetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QStringList headers = {tr("Id"), tr("FileName")};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headers.value(section);
    }
    return {};
}
