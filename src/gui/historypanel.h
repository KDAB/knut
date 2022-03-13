#pragma once

#include <QTreeView>

namespace Gui {

class HistoryPanel : public QTreeView
{
    Q_OBJECT
public:
    explicit HistoryPanel(QWidget *parent = nullptr);

    QWidget *toolBar() const;

private:
    QWidget *const m_toolBar = nullptr;
};

} // namespace Gui
