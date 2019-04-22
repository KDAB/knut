#ifndef CONTENTTREE_H
#define CONTENTTREE_H

#include <QTreeView>

struct Data;

class ContentTree : public QTreeView
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

private:
    QAbstractItemModel *m_model = nullptr;
    Data *m_data = nullptr;
};

#endif // CONTENTTREE_H
