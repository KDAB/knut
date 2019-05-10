#ifndef ACCELERATORMODEL_H
#define ACCELERATORMODEL_H

#include "data.h"

#include <QAbstractTableModel>

class AcceleratorModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns {
        ID,
        Shortcut,
    };

public:
    explicit AcceleratorModel(const Data::AcceleratorTable &table, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    const QVector<Data::Accelerator> &m_accelerators;
};

#endif // ACCELERATORMODEL_H
