#pragma once

#include <QTreeView>

namespace Core {
class HistoryModel;
}

namespace Gui {

class HistoryPanel : public QTreeView
{
    Q_OBJECT
public:
    explicit HistoryPanel(QWidget *parent = nullptr);

    QWidget *toolBar() const;

signals:
    void scriptCreated(const QString &script);

private:
    void createScript();

    QWidget *const m_toolBar = nullptr;
    Core::HistoryModel *const m_model = nullptr;
};

} // namespace Gui
