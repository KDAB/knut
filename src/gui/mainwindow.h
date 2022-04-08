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
class HistoryPanel;
class ScriptPanel;

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
    // File
    void openProject();
    void saveDocument();
    void saveAllDocuments();
    void closeDocument();
    void openOptions();
    void showPalette();
    void returnToEditor();

    // Script
    void runScript();

    // Edit
    void toggleMark();
    void goToMark();
    void selectToMark();

    // C++
    void followSymbol();
    void switchDeclarationDefinition();
    void switchHeaderSource();
    void gotoBlockStart();
    void gotoBlockEnd();
    void selectBlockStart();
    void selectBlockEnd();
    void commentSelection();

    // Rc
    void createQrc();
    void createUi();

    // Help
    void aboutKnut();

    void updateActions();
    void updateScriptActions();

    void initProject(const QString &path);
    void openDocument(const QModelIndex &index);
    void updateRecentProjects();
    void changeTab();
    void changeCurrentDocument();
    void createDock(QWidget *widget, Qt::DockWidgetArea area, QWidget *toolbar = nullptr);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QMenu *m_recentProjects = nullptr;
    QFileSystemModel *const m_fileModel = nullptr;
    QTreeView *const m_projectView = nullptr;
    Palette *const m_palette = nullptr;
    HistoryPanel *const m_historyPanel = nullptr;
    ScriptPanel *const m_scriptPanel = nullptr;
};

} // namespace Gui
