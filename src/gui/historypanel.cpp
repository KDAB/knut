#include "historypanel.h"

#include "guisettings.h"

#include "core/logger.h"

#include <QAction>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QToolButton>

namespace Gui {

HistoryPanel::HistoryPanel(QWidget *parent)
    : QTreeView(parent)
    , m_toolBar(new QWidget)
    , m_clearButton(new QToolButton(m_toolBar))
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

    GuiSettings::setIcon(m_clearButton, ":/gui/delete-sweep.png");
    m_clearButton->setToolTip(tr("Clear"));
    m_clearButton->setAutoRaise(true);
    layout->addWidget(m_clearButton);
    connect(m_clearButton, &QToolButton::clicked, m_model, &Core::HistoryModel::clear);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ContiguousSelection);
    setContextMenuPolicy(Qt::ActionsContextMenu);

    auto action = new QAction(tr("Create Script"), this);
    connect(action, &QAction::triggered, this, &HistoryPanel::createScriptFromSelection);
    addAction(action);
}

QWidget *HistoryPanel::toolBar() const
{
    return m_toolBar;
}

bool HistoryPanel::isRecording() const
{
    return m_startRow != -1;
}

void HistoryPanel::startRecording()
{
    m_startRow = m_model->rowCount();
    m_clearButton->setEnabled(false);
    emit recordingChanged(true);
}

void HistoryPanel::stopRecording()
{
    emit scriptCreated(m_model->createScript(m_startRow, m_model->rowCount() - 1));
    m_clearButton->setEnabled(true);
    m_startRow = -1;
    emit recordingChanged(false);
}

void HistoryPanel::createScriptFromSelection()
{
    auto selection = selectionModel()->selectedIndexes();
    emit scriptCreated(m_model->createScript(selection.first(), selection.last()));
}

} // namespace Gui
