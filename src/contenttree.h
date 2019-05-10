#ifndef CONTENTTREE_H
#define CONTENTTREE_H

#include "data.h"

#include <QWidget>

struct Data;
class QAbstractItemModel;
class QTreeView;
class QTreeWidget;

class ContentTree : public QWidget
{
    Q_OBJECT

public:
    explicit ContentTree(QWidget *parent = nullptr);

    void setResourceData(Data *file);
    void setData(int type, int index);
    void clear();

signals:
    void rcLineChanged(int line);

private:
    void changeCurrentItem(const QModelIndex &current);
    void updateDialogProperty(const Data::Dialog &dialog);

private:
    QTreeView *m_contentView;
    QTreeWidget *m_propertyView;
    QAbstractItemModel *m_model = nullptr;
    Data *m_data = nullptr;
};

#endif // CONTENTTREE_H
