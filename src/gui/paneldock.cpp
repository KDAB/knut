#include "paneldock.h"

#include "abstractpanel.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QToolButton>

namespace Gui {

PanelDock::PanelDock(QWidget *parent)
    : QDockWidget(parent)
    , m_panelStack(new QStackedWidget(this))
    , m_toolBarStack(new QStackedWidget(this))
    , m_panelButtonGroup(new QButtonGroup(this))
{
    setAllowedAreas(Qt::BottomDockWidgetArea);
    setFeatures(QDockWidget::NoDockWidgetFeatures);
    setObjectName("PanelDock");
    setWidget(m_panelStack);

    auto titleBar = new QWidget(this);
    auto layout = new QHBoxLayout(titleBar);
    layout->setContentsMargins({});
    layout->addStretch(1);
    layout->addWidget(m_toolBarStack);
    setTitleBarWidget(titleBar);

    connect(m_panelButtonGroup, &QButtonGroup::idClicked, m_panelStack, &QStackedWidget::setCurrentIndex);
    connect(m_panelButtonGroup, &QButtonGroup::idClicked, m_toolBarStack, &QStackedWidget::setCurrentIndex);
}

PanelDock::~PanelDock() = default;

void PanelDock::addPanel(std::unique_ptr<AbstractPanel> panel)
{
    m_panelStack->addWidget(panel->widget());
    m_toolBarStack->addWidget(panel->toolBar());

    auto panelButton = new QToolButton(this);
    panelButton->setText(panel->title());
    panelButton->setAutoRaise(true);
    panelButton->setCheckable(true);
    panelButton->setChecked(m_panels.empty());

    const int id = static_cast<int>(m_panels.size());
    m_panelButtonGroup->addButton(panelButton, id);
    static_cast<QHBoxLayout *>(titleBarWidget()->layout())->insertWidget(id, panelButton);

    m_panels.push_back(std::move(panel));
}

} // namespace Gui
