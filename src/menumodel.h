#ifndef MENUMODEL_H
#define MENUMODEL_H

#include "data.h"

#include <QAbstractItemModel>
#include <QHash>

class MenuModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Columns {
        Title,
        Checkable,
        ID,
        Shortcut,
        ToolTip,
        StatusTip,
        ColumnCount,
    };

public:
    explicit MenuModel(Data *file, Data::MenuItem *menu, QObject *parent = nullptr);

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    Data::MenuItem *indexToMenu(const QModelIndex &index) const;

private:
    Data *m_data;
    Data::MenuItem *m_menu;

    QHash<Data::MenuItem *, Data::MenuItem *> m_parentMap;
    QHash<Data::MenuItem *, int> m_indexMap;
};

#endif // MENUMODEL_H
