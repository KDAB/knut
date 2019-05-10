#ifndef TOOLBARMODEL_H
#define TOOLBARMODEL_H

#include "data.h"

#include <QAbstractListModel>

class ToolBarModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Columns {
        ID,
        Shortcut,
    };

public:
    explicit ToolBarModel(const Data::ToolBar &toolBar, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    const QVector<Data::ToolBarItem> &m_items;
};

#endif // TOOLBARMODEL_H
