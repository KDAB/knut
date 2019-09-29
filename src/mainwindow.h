#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "actiondialog.h"
#include "data.h"
#include "global.h"
#include "menudialog.h"
#include "toolbardialog.h"
#include "widgetdialog.h"

#include <QMainWindow>
#include <QPointer>

class OverviewModel;

namespace Ui {
class MainWindow;
}
class QMenu;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void highlightLine(int line);

private:
    void openData();
    void extractActions();
    void extractMenus();
    void closeFile();
    void updateRecentFileActions();
    void openFile(const QString &fileName);
    void updateRecentFiles(const QString &fileName);
    void extractToolbars();
    void extractWidgets();
    void previewDialog(const QModelIndex &index);

private:
    Ui::MainWindow *ui = nullptr;
    QMenu *m_recentMenu = nullptr;
    Data m_data;
    QPointer<ActionDialog> m_actionDialog;
    QPointer<MenuDialog> m_menuDialog;
    QPointer<WidgetDialog> m_widgetDialog;
    QPointer<ToolbarDialog> m_toolbarDialog;
};

#endif // MAINWINDOW_H
