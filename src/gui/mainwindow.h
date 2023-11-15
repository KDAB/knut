#pragma once

#include <QMainWindow>

#include <memory>

class QMenu;
class QFileSystemModel;
class QTreeView;

namespace Gui {

class Palette;
class HistoryPanel;
class ScriptPanel;
class DocumentPalette;
class ShortcutManager;
class TreeSitterInspector;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    QList<QAction *> menuActions() const;
    ShortcutManager *shortcutManager() const;

    bool eventFilter(QObject *watched, QEvent *event) override;

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
    void returnToEditor();

    // Script
    void runScript();

    // Edit
    void selectAll();
    void toggleMark();
    void goToMark();
    void selectToMark();
    void deleteLine();
    void undo();
    void redo();

    // C++
    void followSymbol();
    void switchDeclarationDefinition();
    void switchHeaderSource();
    void gotoBlockStart();
    void gotoBlockEnd();
    void selectBlockStart();
    void selectBlockEnd();
    void selectBlockUp();
    void commentSelection();
    void toggleSection();
    void inspectTreeSitter();
    void deleteMethod();

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
    QDockWidget *createDock(QWidget *widget, Qt::DockWidgetArea area, QWidget *toolbar = nullptr);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QMenu *m_recentProjects = nullptr;
    QFileSystemModel *const m_fileModel = nullptr;
    QTreeView *const m_projectView = nullptr;
    Palette *const m_palette = nullptr;
    HistoryPanel *const m_historyPanel = nullptr;
    ScriptPanel *const m_scriptPanel = nullptr;
    DocumentPalette *const m_documentPalette = nullptr;
    ShortcutManager *const m_shortcutManager = nullptr;
    TreeSitterInspector *const m_treeSitterInspector = nullptr;
};

} // namespace Gui
