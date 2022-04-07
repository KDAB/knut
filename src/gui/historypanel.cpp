#include "historypanel.h"

#include "guisettings.h"

#include "core/logger.h"

#include <QAction>
#include <QDebug>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QToolButton>

namespace Gui {

HistoryPanel::HistoryPanel(QWidget *parent)
    : QTreeView(parent)
    , m_toolBar(new QWidget)
    , m_model(new Core::HistoryModel(this))

{
    setWindowTitle(tr("History"));
    setObjectName("HistoryPanel");

    setModel(m_model);
    header()->setStretchLastSection(true);
    header()->setSectionResizeMode(Core::HistoryModel::NameCol, QHeaderView::ResizeToContents);

    auto showLast = [this]() {
        scrollTo(m_model->index(m_model->rowCount() - 1, 0));
    };
    connect(m_model, &QAbstractItemModel::rowsInserted, this, showLast);

    auto layout = new QHBoxLayout(m_toolBar);
    layout->setContentsMargins({});

    auto clearButton = new QToolButton(m_toolBar);
    GuiSettings::setIcon(clearButton, ":/gui/delete-sweep.png");
    clearButton->setToolTip(tr("Clear"));
    clearButton->setAutoRaise(true);
    layout->addWidget(clearButton);
    connect(clearButton, &QToolButton::clicked, m_model, &Core::HistoryModel::clear);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ContiguousSelection);
    setContextMenuPolicy(Qt::ActionsContextMenu);

    auto action = new QAction(tr("Create script"));
    connect(action, &QAction::triggered, this, &HistoryPanel::createScript);
    addAction(action);
}

QWidget *HistoryPanel::toolBar() const
{
    return m_toolBar;
}

void HistoryPanel::createScript()
{
    auto selection = selectionModel()->selectedIndexes();
    emit scriptCreated(m_model->createScript(selection.first(), selection.last()));
}

} // namespace Gui
