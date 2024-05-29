#pragma once

#include <QWidget>
#include <memory>

class QSortFilterProxyModel;
class QAbstractItemModel;
class QTreeView;
class QPlainTextEdit;

namespace RcCore {
struct Data;
struct RcFile;
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
    ~RcFileView() override;

    void setRcFile(const RcCore::RcFile &rcFile);

    QPlainTextEdit *textEdit() const;

signals:
    void languageChanged(const QString &language);

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
    std::unique_ptr<Ui::RcFileView> ui;
    const RcCore::RcFile *m_rcFile = nullptr;
    QSortFilterProxyModel *const m_dataProxyModel;
    QSortFilterProxyModel *const m_contentProxyModel;
    QAbstractItemModel *m_contentModel = nullptr;
};

} // namespace RcUi
