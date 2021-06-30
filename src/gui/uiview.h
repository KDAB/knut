#pragma once

#include <QSplitter>

class QTableView;
class QMdiArea;

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
    QTableView *m_tableView = nullptr;
    QMdiArea *m_previewArea = nullptr;
};

} // namespace Gui
