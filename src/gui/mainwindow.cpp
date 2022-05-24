#include "mainwindow.h"
#include "core/cppdocument.h"
#include "core/lspdocument.h"
#include "ui_mainwindow.h"

#include "documentpalette.h"
#include "guisettings.h"
#include "historypanel.h"
#include "imageview.h"
#include "interfacesettings.h"
#include "logpanel.h"
#include "optionsdialog.h"
#include "palette.h"
#include "rctoqrcdialog.h"
#include "rctouidialog.h"
#include "runscriptdialog.h"
#include "scriptpanel.h"
#include "shortcutmanager.h"
#include "shortcutsettings.h"
#include "textview.h"
#include "uiview.h"

#include "core/cppdocument.h"
#include "core/document.h"
#include "core/imagedocument.h"
#include "core/logger.h"
#include "core/project.h"
#include "core/rcdocument.h"
#include "core/scriptmanager.h"
#include "core/textdocument.h"
#include "core/uidocument.h"
#include "rcui/rcfileview.h"

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
#include <QSettings>
#include <QShortcut>
#include <QToolButton>
#include <QTreeView>

namespace Gui {

constexpr int MaximumRecentProjects = 10;
constexpr char RecentProjectKey[] = "RecentProject";
constexpr char GeometryKey[] = "MainWindow/Geometry";
constexpr char WindowStateKey[] = "MainWindow/WindowState";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_fileModel(new QFileSystemModel(this))
    , m_projectView(new QTreeView(this))
    , m_palette(new Palette(this))
    , m_historyPanel(new HistoryPanel(this))
    , m_scriptPanel(new ScriptPanel(this))
    , m_documentPalette(new DocumentPalette(this))
    , m_shortcutManager(new ShortcutManager(this))
{
    // Initialize the settings before anything
    GuiSettings::instance();

    setAttribute(Qt::WA_DeleteOnClose);

    m_palette->hide();
    m_documentPalette->hide();

    ui->setupUi(this);
    setWindowTitle(QApplication::applicationName() + ' ' + QApplication::applicationVersion());

    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    m_projectView->header()->hide();
    m_projectView->setWindowTitle(tr("Project"));
    m_projectView->setObjectName("Project");
    createDock(m_projectView, Qt::LeftDockWidgetArea);

    auto logPanel = new LogPanel(this);
    createDock(logPanel, Qt::BottomDockWidgetArea, logPanel->toolBar());
    createDock(m_historyPanel, Qt::BottomDockWidgetArea, m_historyPanel->toolBar());
    createDock(m_scriptPanel, Qt::LeftDockWidgetArea, m_scriptPanel->toolBar());

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
    connect(ui->actionRunScript, &QAction::triggered, this, &MainWindow::runScript);
    connect(ui->actionStartRecordingScript, &QAction::triggered, m_historyPanel, &HistoryPanel::startRecording);
    connect(ui->actionStopRecordingScript, &QAction::triggered, m_historyPanel, &HistoryPanel::stopRecording);
    connect(ui->actionPlayLastScript, &QAction::triggered, m_scriptPanel, &ScriptPanel::playScript);

    // Edit
    ui->findWidget->hide();
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
    connect(ui->actionSwitchHeaderSource, &QAction::triggered, this, &MainWindow::switchHeaderSource);
    connect(ui->actionFollowSymbol, &QAction::triggered, this, &MainWindow::followSymbol);
    connect(ui->actionSwitchDeclDef, &QAction::triggered, this, &MainWindow::switchDeclarationDefinition);
    connect(ui->actionCommentSelection, &QAction::triggered, this, &MainWindow::commentSelection);
    connect(ui->actionToggleSection, &QAction::triggered, this, &MainWindow::toggleSection);
    connect(ui->actionGotoBlockEnd, &QAction::triggered, this, &MainWindow::gotoBlockEnd);
    connect(ui->actionGotoBlockStart, &QAction::triggered, this, &MainWindow::gotoBlockStart);
    connect(ui->actionSelectToBlockEnd, &QAction::triggered, this, &MainWindow::selectBlockEnd);
    connect(ui->actionSelectToBlockStart, &QAction::triggered, this, &MainWindow::selectBlockStart);
    connect(ui->actionTransformSymbol, &QAction::triggered, this, &MainWindow::transformSymbol);
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

    auto path = project->root();
    if (!path.isEmpty())
        initProject(path);
    if (project->currentDocument())
        changeCurrentDocument();

    updateActions();
    updateScriptActions();
}

static void actionsFromMenu(QMenu *menu, QList<QAction *> &actions)
{
    const auto &menuActions = menu->actions();
    for (QAction *action : menuActions) {
        if (action->isSeparator())
            continue;
        else if (action->menu()) {
            if (action->menu()->objectName() == "recentProjectsMenu")
                continue;
            actionsFromMenu(action->menu(), actions);
        } else
            actions.push_back(action);
    }
}

QList<QAction *> MainWindow::menuActions() const
{
    QList<QAction *> actions;
    const auto &menus = menuBar()->findChildren<QMenu *>();
    for (QMenu *menu : menus)
        actionsFromMenu(menu, actions);
    actions.append(QMainWindow::actions());
    return actions;
}

ShortcutManager *MainWindow::shortcutManager() const
{
    return m_shortcutManager;
}

void MainWindow::switchHeaderSource()
{
    if (auto cppDocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->currentDocument()))
        cppDocument->openHeaderSource();
}

void MainWindow::gotoBlockStart()
{
    if (auto cppDocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->currentDocument()))
        cppDocument->gotoBlockStart();
}

void MainWindow::gotoBlockEnd()
{
    if (auto cppDocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->currentDocument()))
        cppDocument->gotoBlockEnd();
}

void MainWindow::selectBlockStart()
{
    if (auto cppDocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->currentDocument()))
        cppDocument->selectBlockStart();
}

void MainWindow::selectBlockEnd()
{
    if (auto cppDocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->currentDocument()))
        cppDocument->selectBlockEnd();
}

void MainWindow::commentSelection()
{
    if (auto cppDocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->currentDocument()))
        cppDocument->commentSelection();
}

void MainWindow::toggleSection()
{
    if (auto cppDocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->currentDocument()))
        cppDocument->toggleSection();
}

void MainWindow::transformSymbol()
{
    if (qobject_cast<Core::LspDocument *>(Core::Project::instance()->currentDocument())) {
        auto *scriptManager = Core::ScriptManager::instance();

        scriptManager->runScript(":/scripts/qml/transform-symbol.qml");
    }
}

void MainWindow::deleteMethod()
{
    if (auto cppDocument = qobject_cast<Core::CppDocument *>(Core::Project::instance()->currentDocument()))
        cppDocument->deleteMethod();
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
    auto path = QFileDialog::getExistingDirectory(this, "Open project", QDir::currentPath());
    if (!path.isEmpty()) {
        Core::Project::instance()->setRoot(path);
        initProject(path);
    }
}

void MainWindow::initProject(const QString &path)
{
    // Update recent list
    QSettings settings;
    QStringList projects = settings.value(RecentProjectKey).toStringList();
    projects.removeAll(path);
    projects.prepend(path);
    while (projects.size() > MaximumRecentProjects)
        projects.removeLast();
    settings.setValue(RecentProjectKey, projects);

    // Initalize tree view
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

void MainWindow::createDock(QWidget *widget, Qt::DockWidgetArea area, QWidget *toolbar)
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
            return;
        }
    }
}
void MainWindow::followSymbol()
{
    auto *project = Core::Project::instance();
    auto *document = qobject_cast<Core::LspDocument *>(project->currentDocument());

    if (document) {
        document->followSymbol();
    }
}

void MainWindow::switchDeclarationDefinition()
{
    auto *project = Core::Project::instance();
    auto *document = qobject_cast<Core::LspDocument *>(project->currentDocument());

    if (document) {
        document->switchDeclarationDefinition();
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
Knut version %1<br/><br/>

Knut name has nothing to do with Knut Irvin, nor with Knut the polar bear.<br/>
The name Knut is coming from St Knut, which marks the end of the Christmas and holiday season in Sweden.<br/>
See Wikipedia article: <a href="https://en.wikipedia.org/wiki/Saint_Knut%27s_Day">Saint Knut's Day</a>.)")
                    .arg(KNUT_VERSION);
    QMessageBox dialog(QMessageBox::Information, tr("About Knut"), text, QMessageBox::Ok, this);
    dialog.setIconPixmap(QPixmap(":/gui/icons/knut-64.png"));
    dialog.exec();
}

void MainWindow::runScript()
{
    RunScriptDialog dialog(this);
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
    auto document = Core::Project::instance()->currentDocument();

    ui->actionCloseDocument->setEnabled(document != nullptr);

    auto *textDocument = qobject_cast<Core::TextDocument *>(document);
    ui->actionSelectAll->setEnabled(textDocument != nullptr);
    ui->actionFind->setEnabled(textDocument != nullptr);
    ui->actionReplace->setEnabled(textDocument != nullptr);
    ui->actionFindNext->setEnabled(textDocument != nullptr);
    ui->actionFindPrevious->setEnabled(textDocument != nullptr);
    ui->actionDeleteLine->setEnabled(textDocument != nullptr);
    ui->actionUndo->setEnabled(textDocument != nullptr);
    ui->actionRedo->setEnabled(textDocument != nullptr);
    auto *textView = textViewForDocument(textDocument);
    ui->actionToggleMark->setEnabled(textDocument != nullptr);
    ui->actionGotoMark->setEnabled(textDocument != nullptr && textView->hasMark());
    ui->actionSelectToMark->setEnabled(textDocument != nullptr && textView->hasMark());

    auto *lspDocument = qobject_cast<Core::LspDocument *>(document);
    const bool lspEnabled = lspDocument && lspDocument->hasLspClient();
    ui->actionFollowSymbol->setEnabled(lspEnabled);
    ui->actionSwitchDeclDef->setEnabled(lspEnabled);
    ui->actionTransformSymbol->setEnabled(lspEnabled);

    const bool cppEnabled = lspDocument && qobject_cast<Core::CppDocument *>(document);
    ui->actionSwitchHeaderSource->setEnabled(cppEnabled);
    ui->actionCommentSelection->setEnabled(cppEnabled);
    ui->actionToggleSection->setEnabled(cppEnabled);
    ui->actionGotoBlockEnd->setEnabled(cppEnabled);
    ui->actionGotoBlockStart->setEnabled(cppEnabled);
    ui->actionSelectToBlockEnd->setEnabled(cppEnabled);
    ui->actionSelectToBlockStart->setEnabled(cppEnabled);
    ui->actionDeleteMethod->setEnabled(cppEnabled);

    const bool rcEnabled = qobject_cast<Core::RcDocument *>(document);
    ui->actionCreateQrc->setEnabled(rcEnabled);
    ui->actionCreateUi->setEnabled(rcEnabled);
}

void MainWindow::updateScriptActions()
{
    ui->actionStartRecordingScript->setEnabled(!m_historyPanel->isRecording());
    ui->actionStopRecordingScript->setEnabled(m_historyPanel->isRecording());
    ui->actionPlayLastScript->setEnabled(!m_historyPanel->isRecording() && m_scriptPanel->hasScript());
}

void MainWindow::returnToEditor()
{
    if (ui->tabWidget->currentWidget()->hasFocus())
        ui->findWidget->hide();
    else
        ui->tabWidget->currentWidget()->setFocus(Qt::FocusReason::ShortcutFocusReason);
}

void MainWindow::selectAll()
{
    if (auto textDocument = qobject_cast<Core::TextDocument *>(Core::Project::instance()->currentDocument()))
        textDocument->selectAll();
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

void MainWindow::deleteLine()
{
    if (auto textDocument = qobject_cast<Core::TextDocument *>(Core::Project::instance()->currentDocument()))
        textDocument->deleteLine();
}

void MainWindow::undo()
{
    if (auto textDocument = qobject_cast<Core::TextDocument *>(Core::Project::instance()->currentDocument()))
        textDocument->undo();
}

void MainWindow::redo()
{
    if (auto textDocument = qobject_cast<Core::TextDocument *>(Core::Project::instance()->currentDocument()))
        textDocument->redo();
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

static QWidget *widgetForDocument(Core::Document *document)
{
    switch (document->type()) {
    case Core::Document::Type::Cpp:
    case Core::Document::Type::Text: {
        auto textView = new TextView();
        textView->setTextDocument(qobject_cast<Core::TextDocument *>(document));
        return textView;
    }
    case Core::Document::Type::Rc: {
        auto rcview = new RcUi::RcFileView();
        rcview->setRcFile(qobject_cast<Core::RcDocument *>(document)->data());
        GuiSettings::setupDocumentTextEdit(rcview->textEdit(), document->fileName());
        return rcview;
    }
    case Core::Document::Type::Ui: {
        auto uiview = new UiView();
        uiview->setUiDocument(qobject_cast<Core::UiDocument *>(document));
        return uiview;
    }
    case Core::Document::Type::Image: {
        auto imageview = new ImageView();
        imageview->setImageDocument(qobject_cast<Core::ImageDocument *>(document));
        return imageview;
    }
    }
    Q_UNREACHABLE();
    return nullptr;
}

static void updateTabTitle(QTabWidget *tabWidget, int index, bool hasChanged)
{
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
        QDir dir(project->root());
        auto widget = widgetForDocument(document);
        widget->setWindowTitle(fileName);
        windowIndex = ui->tabWidget->addTab(widget, dir.relativeFilePath(fileName));

        connect(document, &Core::Document::hasChangedChanged, this, [this, windowIndex, document]() {
            updateTabTitle(ui->tabWidget, windowIndex, document->hasChanged());
        });
    }
    ui->tabWidget->setCurrentIndex(windowIndex);
    if (ui->tabWidget->currentWidget())
        ui->tabWidget->currentWidget()->setFocus(Qt::OtherFocusReason);

    const QModelIndex &index = m_fileModel->index(fileName);
    m_projectView->setCurrentIndex(index);
    updateActions();
}

} // namespace Gui
