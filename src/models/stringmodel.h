#ifndef STRINGMODEL_H
#define STRINGMODEL_H

#include "data.h"

#include <QAbstractTableModel>
#include <QList>

class StringModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns {
        ID,
        Text,
    };

public:
    explicit StringModel(Data *data, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    QList<Data::String> m_strings;
};

#endif // STRINGMODEL_H
