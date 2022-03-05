#pragma once

#include <QAbstractItemModel>
#include <QFrame>

#include <functional>
#include <memory>
#include <vector>

class QSortFilterProxyModel;

namespace Gui {

namespace Ui {
    class Palette;
}

class Palette : public QFrame
{
    Q_OBJECT

public:
    explicit Palette(QWidget *parent = nullptr);
    ~Palette();

    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void changeText(const QString &text);
    void clickItem(const QModelIndex &index);
    void setSourceModel(QAbstractItemModel *model);
    void updateListHeight();

    void addFileSelector();
    void addLineSelector();
    void addScriptSelector();
    void addSymbolSelector();

    struct Selector
    {
        QString prefix;
        std::unique_ptr<QAbstractItemModel> model;
        std::function<void()> resetFunc;
        std::function<void(const QVariant &)> selectionFunc;
    };

    std::unique_ptr<Ui::Palette> ui;
    std::vector<Selector> m_selectors;
    int m_currentSelector = 0;
    QSortFilterProxyModel *m_proxyModel = nullptr;
};

} // namespace Gui
