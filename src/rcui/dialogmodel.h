#pragma once

#include "rccore/data.h"

#include <QAbstractTableModel>

namespace RcUi {

class DialogModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns {
        Type,
        ID,
        Geometry,
        Text,
        ClassName,
        Styles,
    };

public:
    explicit DialogModel(const RcCore::Data &data, int index, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    const QVector<RcCore::Data::Control> m_controls;
};

} // namespace RcUi
