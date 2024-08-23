/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "mainwindow.h"
#include "codeview.h"
#include "core/codedocument.h"
#include "core/cppdocument.h"
#include "core/document.h"
#include "core/imagedocument.h"
#include "core/jsondocument.h"
#include "core/logger.h"
#include "core/project.h"
#include "core/qmldocument.h"
#include "core/qttsdocument.h"
#include "core/qtuidocument.h"
#include "core/rcdocument.h"
#include "core/slintdocument.h"
#include "core/textdocument.h"
#include "core/version.h"
#include "documentpalette.h"
#include "findinfilespanel.h"
#include "guisettings.h"
#include "historypanel.h"
#include "imageview.h"
#include "interfacesettings.h"
#include "kdalgorithms.h"
#include "logpanel.h"
#include "optionsdialog.h"
#include "palette.h"
#include "qmlview.h"
#include "qttsview.h"
#include "qtuiview.h"
#include "rctoqrcdialog.h"
#include "rctouidialog.h"
#include "rcui/rcfileview.h"
#include "runscriptwidget.h"
#include "scriptlistpanel.h"
#include "scriptpanel.h"
#include "shortcutmanager.h"
#include "shortcutsettings.h"
#include "slintview.h"
#include "textview.h"
#include "toolbar.h"
#include "treesitterinspector.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QScopedValueRollback>
#include <QSettings>
#include <QShortcut>
#include <QToolButton>
#include <QTreeView>

namespace Gui {

constexpr int MaximumRecentProjects = 10;
constexpr char RecentProjectKey[] = "RecentProject";
constexpr char GeometryKey[] = "MainWindow/Geometry";
constexpr char WindowStateKey[] = "MainWindow/WindowState";

// Forward the function with the given FUNCTION name to the current document if it's of the specified TYPE.
// As this was a common pattern, a macro is used to avoid code duplication.
// This generates the appropriate implementation for a member function on MainWindow with the same name.
// The function still needs to be declared in the header.
#define FORWARD_TO_DOCUMENT(TYPE, FUNCTION)                                                                            \
    void MainWindow::FUNCTION()                                                                                        \
    {                                                                                                                  \
        if (auto document = qobject_cast<Core::TYPE *>(Core::Project::instance()->currentDocument()))                  \
            document->FUNCTION();                                                                                      \
    }

FORWARD_TO_DOCUMENT(TextDocument, selectAll)

FORWARD_TO_DOCUMENT(TextDocument, deleteLine)

FORWARD_TO_DOCUMENT(TextDocument, undo)
FORWARD_TO_DOCUMENT(TextDocument, redo)

FORWARD_TO_DOCUMENT(CodeDocument, selectLargerSyntaxNode)
FORWARD_TO_DOCUMENT(CodeDocument, selectSmallerSyntaxNode)
FORWARD_TO_DOCUMENT(CodeDocument, selectNextSyntaxNode)
FORWARD_TO_DOCUMENT(CodeDocument, selectPreviousSyntaxNode)
FORWARD_TO_DOCUMENT(CodeDocument, followSymbol)
FORWARD_TO_DOCUMENT(CodeDocument, switchDeclarationDefinition)

FORWARD_TO_DOCUMENT(CppDocument, openHeaderSource)

FORWARD_TO_DOCUMENT(CppDocument, gotoBlockStart)
FORWARD_TO_DOCUMENT(CppDocument, gotoBlockEnd)
FORWARD_TO_DOCUMENT(CppDocument, selectBlockStart)
FORWARD_TO_DOCUMENT(CppDocument, selectBlockEnd)
FORWARD_TO_DOCUMENT(CppDocument, selectBlockUp)

FORWARD_TO_DOCUMENT(CppDocument, commentSelection)
FORWARD_TO_DOCUMENT(CppDocument, toggleSection)

FORWARD_TO_DOCUMENT(CppDocument, deleteMethod)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_fileModel(new QFileSystemModel(this))
    , m_projectView(new QTreeView(this))
    , m_palette(new Palette(this))
    , m_historyPanel(new HistoryPanel(this))
    , m_scriptPanel(new ScriptPanel(this))
    , m_scriptlistpanel(new ScriptListPanel(this))
    , m_documentPalette(new DocumentPalette(this))
    , m_shortcutManager(new ShortcutManager(this))
    , m_toolBar(new ToolBar(this))
{
    // Initialize the settings before anything
    GuiSettings::instance();

    setAttribute(Qt::WA_DeleteOnClose);

    m_palette->hide();
    m_documentPalette->hide();

    ui->setupUi(this);
    setWindowTitle(generateWindowTitle());
    ui->tabWidget->setCornerWidget(m_toolBar);

    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    m_projectView->header()->hide();
    m_projectView->setWindowTitle(tr("Project"));
    m_projectView->setObjectName("Project");
    createDock(m_projectView, Qt::LeftDockWidgetArea);

    auto logPanel = new LogPanel(this);
    createDock(logPanel, Qt::BottomDockWidgetArea, logPanel->toolBar());
    createDock(m_historyPanel, Qt::BottomDockWidgetArea, m_historyPanel->toolBar());
    auto findInFilesPanel = new FindInFilesPanel(this);
    createDock(findInFilesPanel, Qt::BottomDockWidgetArea, findInFilesPanel->toolBar());
    auto scriptDock = createDock(m_scriptPanel, Qt::LeftDockWidgetArea, m_scriptPanel->toolBar());
    auto scriptListDock = createDock(m_scriptlistpanel, Qt::BottomDockWidgetArea, m_scriptlistpanel->toolBar());
    scriptListDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_scriptlistpanel->setModel(Core::ScriptManager::model());

    // Ensure we display the script panel when a script is created
    auto showScriptPanel = [scriptDock]() {
        scriptDock->activateWindow();
        scriptDock->show();
        scriptDock->raise();
    };
    connect(m_scriptPanel, &ScriptPanel::newScriptCreated, scriptDock, showScriptPanel);

    connect(m_historyPanel, &HistoryPanel::scriptCreated, m_scriptPanel, &ScriptPanel::setNewScript);
    connect(m_historyPanel, &HistoryPanel::recordingChanged, m_scriptPanel, &ScriptPanel::setDisabled);
    connect(m_historyPanel, &HistoryPanel::recordingChanged, m_scriptPanel->toolBar(), &ScriptPanel::setDisabled);
    connect(m_historyPanel, &HistoryPanel::recordingChanged, this, &MainWindow::updateScriptActions);
    connect(m_scriptPanel, &ScriptPanel::textChanged, this, &MainWindow::updateScriptActions);

    // File
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openProject);
    connect(ui->actionOptions, &QAction::triggered, this, &MainWindow::openOptions);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveDocument);
    connect(ui->actionSaveAll, &QAction::triggered, this, &MainWindow::saveAllDocuments);
    connect(ui->actionShowPalette, &QAction::triggered, this, [this]() {
        m_palette->showPalette();
    });
    connect(ui->actionCloseDocument, &QAction::triggered, this, &MainWindow::closeDocument);

    // Script
    ui->runScriptWidget->hide();
    ui->runScriptWidget->installEventFilter(this);
    ui->apiExecutorWidget->hide();
    ui->apiExecutorWidget->installEventFilter(this);
    connect(ui->actionRunScript, &QAction::triggered, ui->runScriptWidget, &RunScriptWidget::open);
    connect(ui->actionStartRecordingScript, &QAction::triggered, m_historyPanel, &HistoryPanel::startRecording);
    connect(ui->actionStopRecordingScript, &QAction::triggered, m_historyPanel, &HistoryPanel::stopRecording);
    connect(ui->actionPlayLastScript, &QAction::triggered, m_scriptPanel, &ScriptPanel::runScript);
    connect(ui->actionExecuteAPI, &QAction::triggered, ui->apiExecutorWidget, &APIExecutorWidget::open);

    // Edit
    ui->findWidget->hide();
    ui->findWidget->installEventFilter(this);
    connectFindWidget();
    connect(ui->actionSelectAll, &QAction::triggered, this, &MainWindow::selectAll);
    connect(ui->actionFind, &QAction::triggered, ui->findWidget, &FindWidget::open);
    connect(ui->actionReplace, &QAction::triggered, ui->findWidget, &FindWidget::open);
    connect(ui->actionFindNext, &QAction::triggered, ui->findWidget, &FindWidget::findNext);
    connect(ui->actionFindPrevious, &QAction::triggered, ui->findWidget, &FindWidget::findPrevious);
    connect(ui->actionToggleMark, &QAction::triggered, this, &MainWindow::toggleMark);
    connect(ui->actionGotoMark, &QAction::triggered, this, &MainWindow::goToMark);
    connect(ui->actionSelectToMark, &QAction::triggered, this, &MainWindow::selectToMark);
    connect(ui->actionDeleteLine, &QAction::triggered, this, &MainWindow::deleteLine);
    connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::undo);
    connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::redo);
    ui->actionRedo->setShortcut(QKeySequence::Redo);
    ui->actionUndo->setShortcut(QKeySequence::Undo);

    // C++
    connect(ui->actionSwitchHeaderSource, &QAction::triggered, this, &MainWindow::openHeaderSource);
    connect(ui->actionFollowSymbol, &QAction::triggered, this, &MainWindow::followSymbol);
    connect(ui->actionSwitchDeclDef, &QAction::triggered, this, &MainWindow::switchDeclarationDefinition);
    connect(ui->actionCommentSelection, &QAction::triggered, this, &MainWindow::commentSelection);
    connect(ui->actionToggleSection, &QAction::triggered, this, &MainWindow::toggleSection);
    connect(ui->actionGotoBlockEnd, &QAction::triggered, this, &MainWindow::gotoBlockEnd);
    connect(ui->actionGotoBlockStart, &QAction::triggered, this, &MainWindow::gotoBlockStart);
    connect(ui->actionSelectToBlockEnd, &QAction::triggered, this, &MainWindow::selectBlockEnd);
    connect(ui->actionSelectToBlockStart, &QAction::triggered, this, &MainWindow::selectBlockStart);
    connect(ui->actionSelectBlockUp, &QAction::triggered, this, &MainWindow::selectBlockUp);
    connect(ui->actionSelectLargerSyntaxNode, &QAction::triggered, this, &MainWindow::selectLargerSyntaxNode);
    connect(ui->actionSelectSmallerSyntaxNode, &QAction::triggered, this, &MainWindow::selectSmallerSyntaxNode);
    connect(ui->actionSelectNextSyntaxNode, &QAction::triggered, this, &MainWindow::selectNextSyntaxNode);
    connect(ui->actionSelectPreviousSyntaxNode, &QAction::triggered, this, &MainWindow::selectPreviousSyntaxNode);
    connect(ui->actionTreeSitterInspector, &QAction::triggered, this, &MainWindow::inspectTreeSitter);
    connect(ui->actionDeleteMethod, &QAction::triggered, this, &MainWindow::deleteMethod);

    // Rc
    connect(ui->actionCreateQrc, &QAction::triggered, this, &MainWindow::createQrc);
    connect(ui->actionCreateUi, &QAction::triggered, this, &MainWindow::createUi);

    // View
    auto showCommandPalette = [this]() {
        m_palette->showPalette(">");
    };
    connect(ui->actionCommandPalette, &QAction::triggered, this, showCommandPalette);

    // About
    connect(ui->actionAboutKnut, &QAction::triggered, this, &MainWindow::aboutKnut);
    connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);

    addAction(ui->actionReturnToEditor);
    connect(ui->actionReturnToEditor, &QAction::triggered, this, &MainWindow::returnToEditor);

    addAction(ui->actionGotoPreviousInHistory);
    connect(ui->actionGotoPreviousInHistory, &QAction::triggered, m_documentPalette, &DocumentPalette::showWindow);

    m_recentProjects = new QMenu(this);
    m_recentProjects->setObjectName("recentProjectsMenu");
    ui->actionRecentProjects->setMenu(m_recentProjects);
    updateRecentProjects();

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::changeTab);

    m_projectView->setModel(m_fileModel);
    for (int i = 1; i < m_fileModel->columnCount(); ++i)
        m_projectView->header()->hideSection(i);

    auto project = Core::Project::instance();
    connect(project, &Core::Project::currentDocumentChanged, this, &MainWindow::changeCurrentDocument);

    auto reloadDocsIfNeeded = [this](Qt::ApplicationState state) {
        if (state == Qt::ApplicationActive)
            reloadDocuments();
    };
    connect(qApp, &QGuiApplication::applicationStateChanged, this, reloadDocsIfNeeded);

    auto path = project->root();
    if (!path.isEmpty())
        initProject(path);
    if (project->currentDocument())
        changeCurrentDocument();

    updateActions();
    updateScriptActions();
}

QList<QAction *> MainWindow::menuActions() const
{
    QList<QAction *> actions;
    const auto &actionMenus = menuBar()->actions();
    for (auto action : actionMenus) {
        if (action->menu()) {
            const auto acts = action->menu()->actions();
            for (const auto &currentAction : acts) {
                if (!currentAction->text().isEmpty()) {
                    actions.append(currentAction);
                }
            }
        }
    }
    actions.append(QMainWindow::actions());
    return actions;
}

QString MainWindow::generateWindowTitle(const QString &projectName) const
{
    QString title = QApplication::applicationName() + ' ' + QApplication::applicationVersion();
    if (!projectName.isEmpty()) {
        title += " (" + projectName + ")";
    }
    return title;
}

ShortcutManager *MainWindow::shortcutManager() const
{
    return m_shortcutManager;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Show) {
        if (qobject_cast<FindWidget *>(watched)) {
            ui->apiExecutorWidget->hide();
            ui->runScriptWidget->hide();
        } else if (qobject_cast<APIExecutorWidget *>(watched)) {
            ui->findWidget->hide();
            ui->runScriptWidget->hide();
        } else if (qobject_cast<RunScriptWidget *>(watched)) {
            ui->apiExecutorWidget->hide();
            ui->findWidget->hide();
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::inspectTreeSitter()
{
    // Lazy-initialize the treesitter inspector.
    // Otherwise, even if it's never opened, it will do relatively expensive updates whenever the user types anything.
    if (!m_treeSitterInspector) {
        m_treeSitterInspector = new TreeSitterInspector(this);
    }
    m_treeSitterInspector->show();
    m_treeSitterInspector->raise();
    m_treeSitterInspector->activateWindow();
}

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent *event)
{
    // We need to close everything, as the document's plaintext edit are going to be deleted by the MainWindow, and the
    // Project destructor will arrive too late.
    Core::Project::instance()->closeAll();

    QSettings settings;
    settings.setValue(GeometryKey, saveGeometry());
    settings.setValue(WindowStateKey, saveState());

    QMainWindow::closeEvent(event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    QSettings settings;
    restoreGeometry(settings.value(GeometryKey).toByteArray());
    restoreState(settings.value(WindowStateKey).toByteArray());
}

void MainWindow::openProject()
{
    auto path = QFileDialog::getExistingDirectory(this, tr("Open project"), QDir::currentPath());
    if (!path.isEmpty()) {
        Core::Project::instance()->setRoot(path);
        initProject(path);
    }
}

void MainWindow::initProject(const QString &path)
{
    setWindowTitle(generateWindowTitle(path));
    // Update recent list
    QSettings settings;
    QStringList projects = settings.value(RecentProjectKey).toStringList();
    projects.removeAll(path);
    projects.prepend(path);
    while (projects.size() > MaximumRecentProjects)
        projects.removeLast();
    settings.setValue(RecentProjectKey, projects);

    // Initialize tree view
    auto index = m_fileModel->setRootPath(path);
    m_projectView->setRootIndex(index);
    connect(m_projectView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::openDocument);

    // Disable menus, we can only load one project - restart Knut if needed
    ui->actionOpen->setEnabled(false);
    ui->actionRecentProjects->setEnabled(false);
}

void MainWindow::updateRecentProjects()
{
    QSettings settings;
    const QStringList projects = settings.value(RecentProjectKey).toStringList();

    const int numRecentProjects = qMin(projects.count(), MaximumRecentProjects);
    m_recentProjects->clear();
    for (int i = 0; i < numRecentProjects; ++i) {
        const QString path = projects.value(i);
        QAction *act = m_recentProjects->addAction(path);
        connect(act, &QAction::triggered, this, [this, path]() {
            Core::Project::instance()->setRoot(path);
            initProject(path);
        });
    }
    ui->actionRecentProjects->setEnabled(numRecentProjects > 0);
}

void MainWindow::openDocument(const QModelIndex &index)
{
    auto path = m_fileModel->filePath(index);
    QFileInfo fi(path);
    if (fi.isFile())
        Core::Project::instance()->open(path);
}

void MainWindow::saveAllDocuments()
{
    Core::Project::instance()->saveAllDocuments();
}

QDockWidget *MainWindow::createDock(QWidget *widget, Qt::DockWidgetArea area, QWidget *toolbar)
{
    Q_ASSERT(!widget->windowTitle().isEmpty());
    Q_ASSERT(!widget->objectName().isEmpty());

    auto dock = new QDockWidget(this);
    dock->setWidget(widget);
    dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    dock->setWindowTitle(widget->windowTitle());
    dock->setObjectName(widget->objectName() + "Dock");

    auto titleBar = new QWidget(dock);
    titleBar->setProperty("panelWidget", true);
    auto layout = new QHBoxLayout(titleBar);
    layout->setContentsMargins({6, 0, 0, 0});
    layout->addWidget(new QLabel(widget->windowTitle()));
    if (toolbar) {
        layout->addSpacing(5 * layout->spacing());
        auto separator = new QFrame(titleBar);
        separator->setFrameShape(QFrame::VLine);
        layout->addWidget(separator);
        layout->addWidget(toolbar);
    }
    layout->addStretch(1);
    auto closeButton = new QToolButton(toolbar);
    GuiSettings::setIcon(closeButton, ":/gui/close.png");
    closeButton->setToolTip(tr("Close"));
    closeButton->setAutoRaise(true);
    layout->addWidget(closeButton);
    connect(closeButton, &QToolButton::clicked, dock, &QDockWidget::close);

    dock->setTitleBarWidget(titleBar);

    addDockWidget(area, dock);
    dock->toggleViewAction()->setObjectName("action" + widget->windowTitle().remove(' '));
    ui->menu_View->addAction(dock->toggleViewAction());

    // Tabify all docks on the same area into the
    const auto dockWidgets = findChildren<QDockWidget *>();
    for (auto dockWidget : dockWidgets) {
        if (dockWidget == dock)
            continue;
        if (dockWidgetArea(dockWidget) == area) {
            tabifyDockWidget(dockWidget, dock);
            dockWidget->raise();
            return dock;
        }
    }
    return dock;
}

void MainWindow::reloadDocuments()
{
    static bool conflictDialogShown = false;
    if (conflictDialogShown)
        return;

    std::vector<Core::Document *> conflictDocs;
    const auto documents = Core::Project::instance()->documents();
    // Reload all documents that have changed on disk
    for (auto *document : documents) {
        if (document->hasChangedOnDisk()) {
            if (document->hasChanged())
                conflictDocs.push_back(document);
            else
                document->reload();
        }
    }

    if (conflictDocs.empty())
        return;
    auto conflictFiles = kdalgorithms::transformed<QStringList>(conflictDocs, &Core::Document::fileName);

    const auto title = conflictFiles.size() == 1 ? "File changed externally" : "Files changed externally";

    const auto message = conflictFiles.size() == 1
        ? QString("%1\n\nThe file has unsaved changes inside this editor and has been changed externally.\n"
                  "Do you want to reload it and lose the changes made in Knut?")
              .arg(conflictFiles.front())
        : QString("%1\n\nThe files have unsaved changes inside this editor and have been changed externally.\n"
                  "Do you want to reload them and lose the changes made in Knut?")
              .arg(conflictFiles.join("\n"));

    QScopedValueRollback rollback(conflictDialogShown, true);
    const auto result = QMessageBox::question(this, title, message);
    if (result == QMessageBox::Yes) {
        for (auto *document : conflictDocs)
            document->reload();
    }
}

void MainWindow::saveDocument()
{
    auto document = Core::Project::instance()->currentDocument();
    if (document)
        document->save();
}

void MainWindow::closeDocument()
{
    auto document = Core::Project::instance()->currentDocument();
    if (document)
        document->close();
    ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
}

void MainWindow::createQrc()
{
    auto document = Core::Project::instance()->currentDocument();
    if (auto rcDocument = qobject_cast<Core::RcDocument *>(document)) {
        RcToQrcDialog dialog(rcDocument, this);
        dialog.exec();
    }
}

void MainWindow::createUi()
{
    auto document = Core::Project::instance()->currentDocument();
    if (auto rcDocument = qobject_cast<Core::RcDocument *>(document)) {
        RcToUiDialog dialog(rcDocument, this);
        dialog.exec();
    }
}

void MainWindow::aboutKnut()
{
    auto text = tr(R"(<h1>About Knut</h1>
Knut version %1<br/>
Build date: %2<br/><br/>

Knut name has nothing to do with Knut Irvin, nor with Knut the polar bear.<br/>
The name Knut is coming from St Knut, which marks the end of the Christmas and holiday season in Sweden.<br/>
See Wikipedia article: <a href="https://en.wikipedia.org/wiki/Saint_Knut%27s_Day">Saint Knut's Day</a>.)")
                    .arg(core::knut_version(), core::knut_build_date());
    QMessageBox dialog(QMessageBox::Information, tr("About Knut"), text, QMessageBox::Ok, this);
    dialog.setIconPixmap(QPixmap(":/gui/icons/knut-64.png"));
    dialog.exec();
}

void MainWindow::openOptions()
{
    OptionsDialog dialog(this);
    dialog.addSettings(new InterfaceSettings(this));
    dialog.addSettings(new ShortcutSettings(this));
    dialog.exec();
}

static TextView *textViewForDocument(Core::Document *document)
{
    if (auto textDocument = qobject_cast<Core::TextDocument *>(document))
        return static_cast<TextView *>(textDocument->textEdit()->parentWidget());
    return nullptr;
}

void MainWindow::updateActions()
{
    auto findInterface = dynamic_cast<FindInterface *>(ui->tabWidget->currentWidget());
    const bool canSearch = findInterface && findInterface->findFlags() & FindInterface::CanSearch;
    const bool canReplace = findInterface && findInterface->findFlags() & FindInterface::CanReplace;

    auto document = Core::Project::instance()->currentDocument();
    auto *textDocument = qobject_cast<Core::TextDocument *>(document);
    const bool isTextDocument = textDocument != nullptr;
    auto *textView = textViewForDocument(textDocument);
    const bool hasMark = textView && textView->hasMark();
    auto *codeDocument = qobject_cast<Core::CodeDocument *>(document);
    const bool lspEnabled = codeDocument && codeDocument->hasLspClient();
    const bool isCodeDocument = codeDocument != nullptr;
    const bool isCppDocument = codeDocument && qobject_cast<Core::CppDocument *>(document);
    const bool isRcDocument = qobject_cast<Core::RcDocument *>(document);

    ui->actionCloseDocument->setEnabled(document != nullptr);
    ui->actionExecuteAPI->setEnabled(document != nullptr);

    ui->actionSelectAll->setEnabled(isTextDocument);
    ui->actionFind->setEnabled(canSearch);
    ui->actionReplace->setEnabled(canReplace);
    ui->actionFindNext->setEnabled(canSearch);
    ui->actionFindPrevious->setEnabled(canSearch);
    ui->actionDeleteLine->setEnabled(isTextDocument);
    ui->actionUndo->setEnabled(isTextDocument);
    ui->actionRedo->setEnabled(isTextDocument);

    ui->actionToggleMark->setEnabled(isTextDocument);
    ui->actionGotoMark->setEnabled(hasMark);
    ui->actionSelectToMark->setEnabled(hasMark);

    ui->actionFollowSymbol->setEnabled(lspEnabled);
    ui->actionSwitchDeclDef->setEnabled(lspEnabled);

    ui->actionSwitchHeaderSource->setEnabled(isCppDocument);
    ui->actionCommentSelection->setEnabled(isCppDocument);
    ui->actionToggleSection->setEnabled(isCppDocument);
    ui->actionGotoBlockEnd->setEnabled(isCppDocument);
    ui->actionGotoBlockStart->setEnabled(isCppDocument);
    ui->actionSelectToBlockEnd->setEnabled(isCppDocument);
    ui->actionSelectToBlockStart->setEnabled(isCppDocument);
    ui->actionSelectBlockUp->setEnabled(isCppDocument);
    ui->actionDeleteMethod->setEnabled(isCppDocument);

    ui->actionSelectLargerSyntaxNode->setEnabled(isCodeDocument);
    ui->actionSelectSmallerSyntaxNode->setEnabled(isCodeDocument);
    ui->actionSelectNextSyntaxNode->setEnabled(isCodeDocument);
    ui->actionSelectPreviousSyntaxNode->setEnabled(isCodeDocument);
    ui->actionTreeSitterInspector->setEnabled(isCodeDocument);

    ui->actionCreateQrc->setEnabled(isRcDocument);
    ui->actionCreateUi->setEnabled(isRcDocument);
}

void MainWindow::updateFindWidgetDisplay()
{
    if (auto findInterface = dynamic_cast<FindInterface *>(ui->tabWidget->currentWidget())) {
        ui->findWidget->setReplaceVisible(findInterface->findFlags() & FindInterface::CanReplace);
    }
}

void MainWindow::updateScriptActions()
{
    ui->actionStartRecordingScript->setEnabled(!m_historyPanel->isRecording());
    ui->actionStopRecordingScript->setEnabled(m_historyPanel->isRecording());
    ui->actionPlayLastScript->setEnabled(!m_historyPanel->isRecording() && m_scriptPanel->hasScript());
}

void MainWindow::connectFindWidget()
{
    auto handleFindRequest = [this](const QString &text, int options) {
        if (auto findInterface = dynamic_cast<FindInterface *>(ui->tabWidget->currentWidget()))
            findInterface->find(text, options);
    };
    connect(ui->findWidget, &FindWidget::findRequested, this, handleFindRequest);

    auto handleCloseRequest = [this]() {
        if (auto findInterface = dynamic_cast<FindInterface *>(ui->tabWidget->currentWidget()))
            findInterface->cancelFind();
    };
    connect(ui->findWidget, &FindWidget::widgetClosed, this, handleCloseRequest);

    auto handleReplace = [this](const QString &before, const QString &after, int options, bool replaceAll) {
        if (auto findInterface = dynamic_cast<FindInterface *>(ui->tabWidget->currentWidget()))
            findInterface->replace(before, after, options, replaceAll);
    };
    connect(ui->findWidget, &FindWidget::replaceRequested, this, handleReplace);
}

void MainWindow::returnToEditor()
{
    if (ui->tabWidget->count() == 0)
        return;

    if (ui->tabWidget->currentWidget()->hasFocus()) {
        ui->findWidget->hide();
        ui->apiExecutorWidget->hide();
    } else
        ui->tabWidget->currentWidget()->setFocus(Qt::FocusReason::ShortcutFocusReason);
}

void MainWindow::toggleMark()
{
    auto document = Core::Project::instance()->currentDocument();
    if (auto *textView = textViewForDocument(document)) {
        textView->toggleMark();
        updateActions();
    }
}

void MainWindow::goToMark()
{
    auto document = Core::Project::instance()->currentDocument();
    if (auto *textView = textViewForDocument(document))
        textView->gotoMark();
}

void MainWindow::selectToMark()
{
    auto document = Core::Project::instance()->currentDocument();
    if (auto *textView = textViewForDocument(document))
        textView->selectToMark();
}

void MainWindow::changeTab()
{
    if (ui->tabWidget->count() == 0) {
        ui->actionCreateQrc->setEnabled(false);
        ui->actionCreateUi->setEnabled(false);
        m_projectView->selectionModel()->clear();
        return;
    }

    auto document = Core::Project::instance()->open(ui->tabWidget->currentWidget()->windowTitle());
    if (!document)
        return;

    ui->actionCreateQrc->setEnabled(document->type() == Core::Document::Type::Rc);
    ui->actionCreateUi->setEnabled(document->type() == Core::Document::Type::Rc);
    ui->actionSwitchHeaderSource->setEnabled(document->type() == Core::Document::Type::Cpp);
}

QWidget *MainWindow::widgetForDocument(Core::Document *document)
{
    switch (document->type()) {
    case Core::Document::Type::Rc: {
        auto rcDocument = qobject_cast<Core::RcDocument *>(document);
        auto rcview = new RcUi::RcFileView();
        rcview->setRcFile(rcDocument->file());
        QObject::connect(rcview, &RcUi::RcFileView::languageChanged, rcDocument, &Core::RcDocument::setLanguage);
        GuiSettings::setupDocumentTextEdit(rcview->textEdit(), document);

        auto updateData = [rcview, rcDocument]() {
            rcview->setRcFile(rcDocument->file());
        };
        QObject::connect(rcDocument, &Core::RcDocument::fileUpdated, rcview, updateData);

        return rcview;
    }
    case Core::Document::Type::QtUi: {
        auto uiview = new QtUiView();
        uiview->setUiDocument(qobject_cast<Core::QtUiDocument *>(document));
        return uiview;
    }
    case Core::Document::Type::Image: {
        auto imageview = new ImageView();
        imageview->setImageDocument(qobject_cast<Core::ImageDocument *>(document));
        return imageview;
    }
    case Core::Document::Type::Slint: {
        auto slintView = new SlintView();
        slintView->setDocument(qobject_cast<Core::SlintDocument *>(document));
        return slintView;
    }
    case Core::Document::Type::Qml: {

        auto qmlview = new QmlView();
        qmlview->setDocument(qobject_cast<Core::QmlDocument *>(document));
        return qmlview;
    }
    case Core::Document::Type::QtTs: {
        auto tsView = new QtTsView();
        tsView->setTsDocument(qobject_cast<Core::QtTsDocument *>(document));
        return tsView;
    }
    case Core::Document::Type::Cpp: {
        auto codeView = new CodeView();
        codeView->setDocument(qobject_cast<Core::CodeDocument *>(document));
        connect(codeView, &CodeView::treeSitterExplorerRequested, this, &MainWindow::inspectTreeSitter);
        return codeView;
    }
    case Core::Document::Type::Json:
    case Core::Document::Type::Text:
    default: {
        auto textView = new TextView();
        textView->setDocument(qobject_cast<Core::TextDocument *>(document));
        return textView;
    }
    }
    Q_UNREACHABLE();
    return nullptr;
}

static void updateTabTitle(QTabWidget *tabWidget, QWidget *widget, bool hasChanged)
{
    const int index = tabWidget->indexOf(widget);
    QString text = tabWidget->tabText(index);
    if (hasChanged && !text.endsWith('*'))
        text += '*';
    else if (!hasChanged && text.endsWith('*'))
        text.remove(text.size() - 1, 1);
    tabWidget->setTabText(index, text);
}

void MainWindow::changeCurrentDocument()
{
    auto project = Core::Project::instance();
    if (!project->currentDocument())
        return;
    const QString fileName = project->currentDocument()->fileName();

    // open the header/source file for C++, so LSP server can also index it
    if (auto cppDocument = qobject_cast<Core::CppDocument *>(project->currentDocument())) {
        Core::LoggerDisabler ld(true);
        project->get(cppDocument->correspondingHeaderSource());
    }

    // find index of the document, if any
    int windowIndex = -1;
    for (int i = 0; i < ui->tabWidget->count(); ++i) {
        if (ui->tabWidget->widget(i)->windowTitle() == fileName) {
            windowIndex = i;
            break;
        }
    }

    // open the window if it's already opened
    if (windowIndex == -1) {
        auto document = project->currentDocument();
        auto widget = widgetForDocument(document);

        widget->setWindowTitle(fileName);
        const auto fi = QFileInfo {fileName};
        windowIndex = ui->tabWidget->addTab(widget, fi.fileName());
        ui->tabWidget->setTabToolTip(windowIndex, fileName);

        connect(document, &Core::Document::hasChangedChanged, this, [this, widget, document]() {
            updateTabTitle(ui->tabWidget, widget, document->hasChanged());
        });
    }
    ui->tabWidget->setCurrentIndex(windowIndex);
    if (auto view = ui->tabWidget->currentWidget()) {
        view->setFocus(Qt::OtherFocusReason);
        m_toolBar->setView(view);
    }

    const QModelIndex &index = m_fileModel->index(fileName);
    m_projectView->setCurrentIndex(index);
    updateActions();
    updateFindWidgetDisplay();
}

} // namespace Gui
