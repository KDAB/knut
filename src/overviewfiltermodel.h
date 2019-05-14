#ifndef OVERVIEWFILTERMODEL_H
#define OVERVIEWFILTERMODEL_H

#include "global.h"

#include <QHash>
#include <QSortFilterProxyModel>

class OverviewFilterModel : public QSortFilterProxyModel
{
public:
    explicit OverviewFilterModel(QObject *parent = nullptr);

    void setExclusive(bool exclusive);
    bool isExclusive() const;

    void setDataType(QVector<Knut::DataType> types);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Knut::DataCollection selectedData() const;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    bool m_exclusive = false;
    QVector<Knut::DataType> m_types;
    QHash<QPair<int, int>, bool> m_checkStates;
};

#endif // OVERVIEWFILTERMODEL_H
