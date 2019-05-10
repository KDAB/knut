#ifndef INCLUDEMODEL_H
#define INCLUDEMODEL_H

#include "data.h"

#include <QAbstractListModel>

class IncludeModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Columns {
        FileName,
    };

public:
    explicit IncludeModel(Data *data, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    const QVector<Data::Include> &m_includes;
};

#endif // INCLUDEMODEL_H
