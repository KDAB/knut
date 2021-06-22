#pragma once

#include "rccore/data.h"

#include <QAbstractTableModel>

namespace RcUi {

class AcceleratorModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns {
        ID,
        Shortcut,
    };

public:
    explicit AcceleratorModel(const RcCore::Data &data, int index, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    const QVector<RcCore::Data::Accelerator> m_accelerators;
};

} // namespace RcUi
