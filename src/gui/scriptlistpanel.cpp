#include "scriptlistpanel.h"

#include "guisettings.h"

#include "core/scriptmanager.h"
#include "core/scriptmodel.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QToolButton>

#include <spdlog/spdlog.h>

namespace Gui {

ScriptListPanel::ScriptListPanel(QWidget *parent)
    : QTreeView(parent)
    , m_toolBar(new QWidget)
{
    setWindowTitle(tr("Script List"));
    setObjectName("ScriptListPanel");

    setUniformRowHeights(true);
    setRootIsDecorated(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(this, &ScriptListPanel::doubleClicked, this, &ScriptListPanel::runScript);

    auto layout = new QHBoxLayout(m_toolBar);
    layout->setContentsMargins({});

    layout->addWidget(new QLabel("Search:"));

    m_filter = new QLineEdit(this);
    layout->addWidget(m_filter);
    m_filter->setPlaceholderText(tr("Filter scripts..."));
    m_filter->setClearButtonEnabled(true);

    auto separator = new QFrame(m_toolBar);
    separator->setFrameShape(QFrame::VLine);
    layout->addWidget(separator);

    m_runButton = new QToolButton(m_toolBar);
    GuiSettings::setIcon(m_runButton, ":/gui/play.png");
    m_runButton->setToolTip(tr("Run"));
    m_runButton->setAutoRaise(true);
    layout->addWidget(m_runButton);

    auto runCurrentScript = [this]() {
        runScript(currentIndex());
    };
    connect(m_runButton, &QToolButton::clicked, this, runCurrentScript);
}

QWidget *ScriptListPanel::toolBar() const
{
    return m_toolBar;
}

void ScriptListPanel::setModel(QAbstractItemModel *model)
{
    Q_ASSERT(model);
    // We can't change the model once it's set
    Q_ASSERT(QTreeView::model() == nullptr);

    auto filterModel = new QSortFilterProxyModel(this);
    filterModel->setSourceModel(model);
    QTreeView::setModel(filterModel);

    connect(m_filter, &QLineEdit::textChanged, filterModel, &QSortFilterProxyModel::setFilterWildcard);

    header()->setSectionResizeMode(Core::ScriptModel::NameColumn, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(Core::ScriptModel::DescriptionColumn, QHeaderView::Stretch);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &ScriptListPanel::updateRunButton);

    updateRunButton();
}

void ScriptListPanel::runScript(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const auto fileName = index.data(Core::ScriptModel::PathRole).toString();

    if (fileName.isEmpty()) {
        spdlog::warn("ScriptListPanel::runScript - fileName is empty");
        return;
    }

    Core::ScriptManager::instance()->runScript(fileName);
}

void ScriptListPanel::updateRunButton()
{
    m_runButton->setEnabled(model() && currentIndex().isValid());
};

} // namespace Gui
