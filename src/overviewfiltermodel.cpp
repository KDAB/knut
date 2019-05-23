#include "overviewfiltermodel.h"

#include "overviewmodel.h"

OverviewFilterModel::OverviewFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setRecursiveFilteringEnabled(true);
}

void OverviewFilterModel::setExclusive(bool exclusive)
{
    m_exclusive = exclusive;
}

bool OverviewFilterModel::isExclusive() const
{
    return m_exclusive;
}

void OverviewFilterModel::setDataType(QVector<Knut::DataType> types)
{
    m_types = std::move(types);
}

Qt::ItemFlags OverviewFilterModel::flags(const QModelIndex &index) const
{
    if (index.parent().isValid())
        return QSortFilterProxyModel::flags(index) | Qt::ItemIsUserCheckable;
    return QSortFilterProxyModel::flags(index);
}

QVariant OverviewFilterModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::CheckStateRole && index.parent().isValid()) {
        const int type = index.data(OverviewModel::TypeRole).toInt();
        const int row = index.data(OverviewModel::IndexRole).toInt();
        bool checked = m_checkStates.value({type, row}, false);
        return checked ? Qt::Checked : Qt::Unchecked;
    }
    return QSortFilterProxyModel::data(index, role);
}

bool OverviewFilterModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole) {
        const bool checked = value.toBool();
        if (checked && m_exclusive && !m_checkStates.empty()) {
            auto it = m_checkStates.begin();
            while (it != m_checkStates.end()) {
                if (it.value()) {
                    it.value() = false;
                    const auto parentIdx = sourceModel()->index(it.key().first, 0);
                    const auto sourceIdx = sourceModel()->index(it.key().second, 0, parentIdx);
                    setData(mapFromSource(sourceIdx), false, Qt::CheckStateRole);
                }
                ++it;
            }
        }
        const int type = idx.data(OverviewModel::TypeRole).toInt();
        const int row = idx.data(OverviewModel::IndexRole).toInt();
        m_checkStates[{type, row}] = checked;
        emit dataChanged(idx, idx, {Qt::CheckStateRole});
        return true;
    }
    return QSortFilterProxyModel::setData(idx, value, role);
}

Knut::DataCollection OverviewFilterModel::selectedData() const
{
    auto collection = m_checkStates.keys(true);
    Knut::sort(collection, [](const auto left, const auto right) {
        return left.first == right.first ? left.second < right.second : left.first < right.first;
    });
    return collection;
}

bool OverviewFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto index = sourceModel()->index(source_row, 0, source_parent);
    const auto data = static_cast<Knut::DataType>(index.data(OverviewModel::TypeRole).toInt());
    return m_types.contains(data);
}
