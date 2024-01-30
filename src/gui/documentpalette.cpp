#include "documentpalette.h"

#include "gui_constants.h"

#include "core/project.h"

#include <QFileInfo>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMenuBar>

#include <algorithm>

namespace Gui {

DocumentPalette::DocumentPalette(QMainWindow *parent)
    : QListWidget(parent)
{
    setWindowFlag(Qt::Popup);
    setFixedWidth(Constants::PaletteWidth);
    setStyleSheet("QListView::item { padding: 5px }");
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Plain);

    auto changeAndHide = [this](const QModelIndex &index) {
        changeCurrentDocument(index.row());
        hide();
    };
    connect(this, &QListView::clicked, this, changeAndHide);
}

void DocumentPalette::showWindow()
{
    fillList();
    if (count() > 1) {
        const int x = (parentWidget()->width() - width()) / 2;
        const int y = qobject_cast<QMainWindow *>(parentWidget())->menuBar()->height() - 1;
        move(parentWidget()->mapToGlobal(QPoint {x, y}));
        show();
        raise();
        setCurrentRow(1);
    }
}

void DocumentPalette::showEvent(QShowEvent *event)
{
    QListWidget::showEvent(event);
    updateListHeight();
}

void DocumentPalette::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        hide();
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        changeCurrentDocument(currentRow());
        hide();
        break;
    case Qt::Key_Tab:
        setCurrentRow(std::min(count() - 1, currentRow() + 1));
        break;
    case Qt::Key_Backtab:
        setCurrentRow(std::max(0, currentRow() - 1));
        break;
    }
    QListWidget::keyPressEvent(event);
}

void DocumentPalette::keyReleaseEvent(QKeyEvent *event)
{
    if (event->modifiers() == 0) {
        changeCurrentDocument(currentRow());
        hide();
    }
    QListWidget::keyReleaseEvent(event);
}

void DocumentPalette::fillList()
{
    clear();
    const auto documents = Core::Project::instance()->documents();

    for (auto document : documents | std::views::reverse) {
        QFileInfo fi(document->fileName());
        addItem(fi.fileName());
    }
}

void DocumentPalette::updateListHeight()
{
    const int rows = std::min(count(), 15);
    Q_ASSERT(rows);
    const auto &rect = visualRect(model()->index(0, 0));
    setFixedHeight(rows * rect.height() + frameWidth() * 2);
}

void DocumentPalette::changeCurrentDocument(int index)
{
    Core::Project::instance()->openPrevious(index);
}

} // namespace Gui
