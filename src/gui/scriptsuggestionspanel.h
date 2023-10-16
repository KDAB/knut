#pragma once

#include <QTreeView>

namespace Core {
class Document;
}
class QToolButton;

namespace Gui {

class IView;

class ScriptSuggestionsPanel : public QTreeView
{

    Q_OBJECT
public:
    explicit ScriptSuggestionsPanel(QWidget *parent = nullptr);

    QWidget *toolBar() const;

    void setModel(QAbstractItemModel *model) override;

private:
    void runScript(const QModelIndex &index);
    void updateRunButton();

    QWidget *const m_toolBar = nullptr;
    QToolButton *m_runButton = nullptr;
};

} // namespace Gui
