/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QMainWindow>
#include <QPointer>
#include <memory>

class QMenu;
class QFileSystemModel;
class QTreeView;

namespace Core {
class Document;
}

namespace Gui {

class Palette;
class HistoryPanel;
class ScriptPanel;
class ScriptListPanel;
class DocumentPalette;
class ShortcutManager;
class TreeSitterInspector;
class ToolBar;

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
    void openHeaderSource();
    void gotoBlockStart();
    void gotoBlockEnd();
    void selectBlockStart();
    void selectBlockEnd();
    void selectBlockUp();
    void commentSelection();
    void toggleSection();
    void inspectTreeSitter();
    void deleteMethod();

    // Code
    void followSymbol();
    void switchDeclarationDefinition();
    void selectLargerSyntaxNode();
    void selectSmallerSyntaxNode();
    void selectNextSyntaxNode();
    void selectPreviousSyntaxNode();

    // Rc
    void createQrc();
    void createUi();

    // Help
    void aboutKnut();

    void updateActions();
    void updateScriptActions();
    void connectFindWidget();
    void updateFindWidgetDisplay();

    void initProject(const QString &path);
    void openDocument(const QModelIndex &index);
    void updateRecentProjects();
    void changeTab();
    QWidget *widgetForDocument(Core::Document *document);
    void changeCurrentDocument();
    QDockWidget *createDock(QWidget *widget, Qt::DockWidgetArea area, QWidget *toolbar = nullptr);
    void reloadDocuments();
    QString generateWindowTitle(const QString &projectName = {}) const;

    std::unique_ptr<Ui::MainWindow> ui;
    QMenu *m_recentProjects = nullptr;
    QFileSystemModel *const m_fileModel = nullptr;
    QTreeView *const m_projectView = nullptr;
    Palette *const m_palette = nullptr;
    HistoryPanel *const m_historyPanel = nullptr;
    ScriptPanel *const m_scriptPanel = nullptr;
    ScriptListPanel *const m_scriptlistpanel = nullptr;
    DocumentPalette *const m_documentPalette = nullptr;
    ShortcutManager *const m_shortcutManager = nullptr;
    ToolBar *const m_toolBar = nullptr;
    QPointer<TreeSitterInspector> m_treeSitterInspector;
};

} // namespace Gui
