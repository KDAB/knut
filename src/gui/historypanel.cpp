#include "historypanel.h"

#include "guisettings.h"

#include "core/logger.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QToolButton>

namespace Gui {

HistoryPanel::HistoryPanel(QWidget *parent)
    : QTreeView(parent)
    , m_toolBar(new QWidget)
{
    setWindowTitle(tr("History"));
    setObjectName("HistoryPanel");

    auto model = new Core::HistoryModel(this);
    setModel(model);
    header()->setStretchLastSection(true);
    header()->setSectionResizeMode(Core::HistoryModel::NameCol, QHeaderView::ResizeToContents);

    auto showLast = [this, model]() {
        scrollTo(model->index(model->rowCount() - 1, 0));
    };
    connect(model, &QAbstractItemModel::rowsInserted, this, showLast);

    auto layout = new QHBoxLayout(m_toolBar);
    layout->setContentsMargins({});

    auto clearButton = new QToolButton(m_toolBar);
    GuiSettings::setIcon(clearButton, ":/gui/delete-sweep.png");
    clearButton->setToolTip(tr("Clear"));
    clearButton->setAutoRaise(true);
    layout->addWidget(clearButton);
    connect(clearButton, &QToolButton::clicked, model, &Core::HistoryModel::clear);
}

QWidget *HistoryPanel::toolBar() const
{
    return m_toolBar;
}

} // namespace Gui
