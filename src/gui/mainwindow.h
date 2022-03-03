#pragma once

#include <QMainWindow>

#include <memory>

class QMenu;
class QFileSystemModel;
class QTreeView;

namespace Core {
class TextDocument;
class RcDocument;
}

namespace Gui {

class Palette;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void openProject();
    void initProject(const QString &path);
    void updateRecentProjects();
    void openDocument(const QModelIndex &index);
    void saveDocument();
    void closeDocument();
    void createQrc();
    void createUi();
    void runScript();
    void openOptions();
    void showPalette();

    void changeTab();
    void changeCurrentDocument();
    void saveAllDocuments();

    void createDock(QWidget *widget, Qt::DockWidgetArea area, QWidget *toolbar = nullptr);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QMenu *m_recentProjects = nullptr;
    QFileSystemModel *const m_fileModel = nullptr;
    QTreeView *const m_projectView = nullptr;
    Palette *const m_palette = nullptr;
};

} // namespace Gui
