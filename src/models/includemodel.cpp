#include "includemodel.h"

#include "global.h"

#include <QColor>

IncludeModel::IncludeModel(Data *data, QObject *parent)
    : QAbstractListModel(parent)
    , m_includes(data->includes)
{
}

int IncludeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_includes.size();
}

QVariant IncludeModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        const auto &include = m_includes.at(index.row());
        return include.fileName;
    }

    if (role == Qt::ForegroundRole) {
        const auto &include = m_includes.at(index.row());
        if (!include.exist)
            return QVariant::fromValue(QColor(Qt::red));
    }

    if (role == Knut::LineRole) {
        const auto &include = m_includes.at(index.row());
        return include.line;
    }

    return {};
}

QVariant IncludeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QStringList headers = {tr("FileName")};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headers.value(section);
    }
    return {};
}