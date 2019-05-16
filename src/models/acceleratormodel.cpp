#include "acceleratormodel.h"

#include "global.h"

#include <QColor>

AcceleratorModel::AcceleratorModel(const Data::AcceleratorTable &table, QObject *parent)
    : QAbstractTableModel(parent)
    , m_accelerators(table.accelerators)
{
}

int AcceleratorModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_accelerators.size();
}

int AcceleratorModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant AcceleratorModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        const auto &accelerator = m_accelerators.at(index.row());
        switch (index.column()) {
        case ID:
            return accelerator.id;
        case Shortcut:
            return accelerator.shortcut;
        }
    }

    if (role == Qt::ForegroundRole) {
        const auto &accelerator = m_accelerators.at(index.row());
        if (accelerator.isUnknown())
            return QVariant::fromValue(QColor(Qt::red));
    }

    if (role == Knut::LineRole) {
        const auto &accelerator = m_accelerators.at(index.row());
        return accelerator.line;
    }

    return {};
}

QVariant AcceleratorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QStringList headers = {tr("Id"), tr("Shortcut")};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headers.value(section);
    }
    return {};
}
