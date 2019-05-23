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

    void setResourceData(Data *data);
    void setData(int type, int index);
    void clear();

Q_SIGNALS:
    void rcLineChanged(int line);

private:
    void changeCurrentItem(const QModelIndex &current);
    void updateDialogProperty(const Data::Dialog &dialog);

private:
    QTreeView *m_contentView = nullptr;
    QTreeWidget *m_propertyView = nullptr;
    QAbstractItemModel *m_model = nullptr;
    Data *m_data = nullptr;
};

#endif // CONTENTTREE_H