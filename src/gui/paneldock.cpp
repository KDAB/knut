#include "paneldock.h"

#include "abstractpanel.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QTabBar>
#include <QToolButton>
#include <QVariant>

namespace Gui {

PanelDock::PanelDock(QWidget *parent)
    : QDockWidget(parent)
    , m_panelStack(new QStackedWidget(this))
    , m_toolBarStack(new QStackedWidget(this))
    , m_panelTab(new QTabBar(this))
{
    setAllowedAreas(Qt::BottomDockWidgetArea);
    setFeatures(QDockWidget::NoDockWidgetFeatures);
    setObjectName("PanelDock");
    setWidget(m_panelStack);

    auto titleBar = new QWidget(this);
    auto layout = new QHBoxLayout(titleBar);
    layout->setContentsMargins({});
    layout->addWidget(m_panelTab);
    layout->addStretch(1);
    layout->addWidget(m_toolBarStack);
    m_toolBarStack->setProperty("panel", true);
    m_panelTab->setDocumentMode(true);
    setTitleBarWidget(titleBar);

    connect(m_panelTab, &QTabBar::currentChanged, m_panelStack, &QStackedWidget::setCurrentIndex);
    connect(m_panelTab, &QTabBar::currentChanged, m_toolBarStack, &QStackedWidget::setCurrentIndex);
}

PanelDock::~PanelDock() = default;

void PanelDock::addPanel(std::unique_ptr<AbstractPanel> panel)
{
    m_panelStack->addWidget(panel->widget());
    m_toolBarStack->addWidget(panel->toolBar());
    m_panelTab->addTab(panel->title());

    m_panels.push_back(std::move(panel));
}

} // namespace Gui
