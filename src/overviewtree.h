#ifndef OVERVIEWTREE_H
#define OVERVIEWTREE_H

#include <QTreeView>

class OverviewModel;
struct Data;

class OverviewTree : public QTreeView
{
    Q_OBJECT
public:
    explicit OverviewTree(QWidget *parent = nullptr);

    void setResourceData(Data *file);
    void updateModel();

signals:
    void rcLineChanged(int line);
    void dataSelected(int type, int index);

private:
    void changeCurrentItem(const QModelIndex &current);

private:
    OverviewModel *m_model;
};

#endif // OVERVIEWTREE_H
