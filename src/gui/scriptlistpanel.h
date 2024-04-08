#pragma once

#include <QTreeView>

namespace Core {
class Document;
}
class QToolButton;
class QLineEdit;

namespace Gui {

class IView;

class ScriptListPanel : public QTreeView
{

    Q_OBJECT
public:
    explicit ScriptListPanel(QWidget *parent = nullptr);

    QWidget *toolBar() const;

    void setModel(QAbstractItemModel *model) override;

private:
    void runScript(const QModelIndex &index);
    void updateRunButton();

    QWidget *const m_toolBar = nullptr;
    QLineEdit *m_filter = nullptr;
    QToolButton *m_runButton = nullptr;
};

} // namespace Gui
