#pragma once

#include "rcfile/data.h"

#include <QAbstractListModel>

namespace RcUi {

class IncludeModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Columns {
        FileName,
    };

public:
    explicit IncludeModel(const RcFile::Data &data, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    const QVector<RcFile::Data::Include> m_includes;
};

} // namespace RcUi
