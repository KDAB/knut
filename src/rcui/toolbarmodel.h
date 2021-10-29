#pragma once

#include "rccore/data.h"

#include <QAbstractListModel>

namespace RcUi {

class ToolBarModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Columns {
        ID,
        Shortcut,
    };

public:
    explicit ToolBarModel(const RcCore::Data &data, int index, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    const QVector<RcCore::ToolBarItem> m_items;
};

} // namespace RcUi
