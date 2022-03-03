#pragma once

#include <QDockWidget>
#include <QWidget>

#include <memory>

class QStackedWidget;
class QTabBar;

namespace Gui {

class AbstractPanel;

class PanelDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit PanelDock(QWidget *parent = nullptr);
    ~PanelDock();

    void addPanel(std::unique_ptr<AbstractPanel> panel);

private:
    std::vector<std::unique_ptr<AbstractPanel>> m_panels;
    QStackedWidget *const m_panelStack;
    QStackedWidget *const m_toolBarStack;
    QTabBar *const m_panelTab;
};

} // namespace Gui
