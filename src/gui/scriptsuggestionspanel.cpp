#include "scriptsuggestionspanel.h"

#include "guisettings.h"
#include "scriptsinpath.h"
#include "scriptsuggestions.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QToolButton>

#include <spdlog/spdlog.h>

namespace Gui {

ScriptSuggestionsPanel::ScriptSuggestionsPanel(QWidget *parent)
    : QTreeView(parent)
    , m_toolBar(new QWidget)
{
    setWindowTitle(tr("Script Suggestions"));
    setObjectName("ScriptSuggestionsPanel");

    setUniformRowHeights(true);
    setRootIsDecorated(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(this, &ScriptSuggestionsPanel::doubleClicked, this, &ScriptSuggestionsPanel::runScript);

    auto layout = new QHBoxLayout(m_toolBar);
    layout->setContentsMargins({});

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

QWidget *ScriptSuggestionsPanel::toolBar() const
{
    return m_toolBar;
}

void ScriptSuggestionsPanel::setModel(QAbstractItemModel *model)
{
    Q_ASSERT(qobject_cast<ScriptSuggestions *>(model));

    QTreeView::setModel(model);
    header()->setSectionResizeMode(ScriptsInPath::NameColumn, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(ScriptsInPath::DescriptionColumn, QHeaderView::Stretch);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &ScriptSuggestionsPanel::updateRunButton);

    updateRunButton();
}

void ScriptSuggestionsPanel::runScript(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    static_cast<ScriptSuggestions *>(model())->run(index);
}

void ScriptSuggestionsPanel::updateRunButton()
{
    m_runButton->setEnabled(model() && currentIndex().isValid());
};

} // namespace Gui
