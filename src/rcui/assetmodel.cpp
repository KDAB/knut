#include "assetmodel.h"

#include "rcviewer_global.h"

#include <QColor>

#include <algorithm>

using namespace RcFile;

namespace RcUi {

AssetModel::AssetModel(const QVector<Asset> &assets, QObject *parent)
    : QAbstractTableModel(parent)
    , m_assets(assets)
{
    std::sort(m_assets.begin(), m_assets.end(), [](const auto &left, const auto &right) {
        return left.id < right.id;
    });
}

int AssetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_assets.size();
}

int AssetModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
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

    if (role == LineRole) {
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

} // namespace RcUi
