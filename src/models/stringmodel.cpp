#include "stringmodel.h"

#include "global.h"

StringModel::StringModel(Data *data, QObject *parent)
    : QAbstractTableModel(parent)
    , m_strings(data->strings.values())
{
    Knut::sort(m_strings, [](const auto &left, const auto &right) { return left.id < right.id; });
}

int StringModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_strings.size();
}

int StringModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant StringModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        const auto &asset = m_strings.at(index.row());
        switch (index.column()) {
        case ID:
            return asset.id;
        case Text:
            return asset.text;
        }
    }

    if (role == Knut::LineRole) {
        const auto &asset = m_strings.at(index.row());
        return asset.line;
    }

    return {};
}

QVariant StringModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QStringList headers = {"Id", "Text"};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headers.value(section);
    }
    return {};
}
