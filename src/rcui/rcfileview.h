#pragma once

#include <QWidget>

class QSortFilterProxyModel;
class QAbstractItemModel;
class QTreeView;

namespace RcCore {
struct Data;
}

namespace RcUi {

namespace Ui {
    class RcFileView;
}

class RcFileView : public QWidget
{
    Q_OBJECT

public:
    explicit RcFileView(QWidget *parent = nullptr);
    ~RcFileView();

    void setRcFile(const RcCore::Data &data);

private:
    void changeDataItem(const QModelIndex &current);
    void changeContentItem(const QModelIndex &current);
    void setData(int type, int index);
    void updateDialogProperty(int index);
    void previewData(const QModelIndex &index);
    void slotContextMenu(QTreeView *treeView, const QPoint &pos);

    void highlightLine(int line);
    void slotSearchText(const QString &text);
    void slotSearchNext();
    void slotSearchPrevious();

    const RcCore::Data &data() const;

private:
    Ui::RcFileView *ui;
    const RcCore::Data *m_data;
    QSortFilterProxyModel *const m_dataProxyModel;
    QSortFilterProxyModel *const m_contentProxyModel;
    QAbstractItemModel *m_contentModel = nullptr;
};

} // namespace RcUi
