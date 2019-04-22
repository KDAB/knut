#ifndef OVERVIEWMODEL_H
#define OVERVIEWMODEL_H

#include <QAbstractItemModel>
#include <QStringList>

struct Data;

class OverviewModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Roles {
        TypeRole = Qt::UserRole,
        IndexRole,
    };

public:
    explicit OverviewModel(QObject *parent = nullptr);

    void setResourceData(Data *data);
    void updateModel();

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    Data *m_data = nullptr;
};

#endif // OVERVIEWMODEL_H
