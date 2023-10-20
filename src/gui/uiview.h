#pragma once

#include <QSplitter>

class QTableView;
class QMdiArea;
class QMdiSubWindow;

namespace Core {
class UiDocument;
}

namespace Gui {

class UiView : public QSplitter
{
    Q_OBJECT
public:
    explicit UiView(QWidget *parent = nullptr);

    void setUiDocument(Core::UiDocument *document);

private:
    void updateView();

    QTableView *m_tableView = nullptr;
    QMdiArea *m_previewArea = nullptr;
    Core::UiDocument *m_document = nullptr;
    QMdiSubWindow *m_previewWindow = nullptr;
};

} // namespace Gui
