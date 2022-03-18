#include "mainwindow.h"
#include "core/cppdocument.h"
#include "core/lspdocument.h"
#include "ui_mainwindow.h"

#include "guisettings.h"
#include "historypanel.h"
#include "imageview.h"
#include "logpanel.h"
#include "optionsdialog.h"
#include "palette.h"
#include "rctoqrcdialog.h"
#include "rctouidialog.h"
#include "runscriptdialog.h"
#include "uiview.h"

#include "core/document.h"
#include "core/imagedocument.h"
#include "core/project.h"
#include "core/rcdocument.h"
#include "core/textdocument.h"
#include "core/uidocument.h"
#include "rcui/rcfileview.h"
#include <core/cppdocument.h>

#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QSettings>
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
{
    // Initialize the settings before anything
    GuiSettings::instance();

    setAttribute(Qt::WA_DeleteOnClose);

    m_palette->hide();

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

    auto historyPanel = new HistoryPanel(this);
    createDock(historyPanel, Qt::BottomDockWidgetArea, historyPanel->toolBar());

    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openProject);
    connect(ui->actionRun_Script, &QAction::triggered, this, &MainWindow::runScript);
    connect(ui->actionOptions, &QAction::triggered, this, &MainWindow::openOptions);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveDocument);
    connect(ui->actionSaveAll, &QAction::triggered, this, &MainWindow::saveAllDocuments);
    connect(ui->actionShow_Palette, &QAction::triggered, this, &MainWindow::showPalette);
    connect(ui->actionClose_Document, &QAction::triggered, this, &MainWindow::closeDocument);

    ui->findWidget->hide();
    connect(ui->actionFind, &QAction::triggered, ui->findWidget, &FindWidget::open);
    connect(ui->actionReplace, &QAction::triggered, ui->findWidget, &FindWidget::open);
    connect(ui->actionFind_Next, &QAction::triggered, ui->findWidget, &FindWidget::findNext);
    connect(ui->actionFind_Previous, &QAction::triggered, ui->findWidget, &FindWidget::findPrevious);

    connect(ui->actionSwitch_Header_Source, &QAction::triggered, this, &MainWindow::switchHeaderSource);
    connect(ui->actionFollow_Symbol, &QAction::triggered, this, &MainWindow::followSymbol);
    connect(ui->actionSwitch_Decl_Def, &QAction::triggered, this, &MainWindow::switchDeclarationDefinition);

    connect(ui->actionCreate_Qrc, &QAction::triggered, this, &MainWindow::createQrc);
    connect(ui->actionCreate_Ui, &QAction::triggered, this, &MainWindow::createUi);

    addAction(ui->actionReturnEditor);
    connect(ui->actionReturnEditor, &QAction::triggered, this, &MainWindow::returnToEditor);

    m_recentProjects = new QMenu(this);
    ui->actionRecent_Projects->setMenu(m_recentProjects);
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
}

void MainWindow::switchHeaderSource()
{
    auto currDoc = qobject_cast<Core::CppDocument *>(Core::Project::instance()->currentDocument());
    if (currDoc != nullptr) {
        currDoc->openHeaderSource();
    }
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
    ui->actionRecent_Projects->setEnabled(false);
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
    ui->actionRecent_Projects->setEnabled(numRecentProjects > 0);
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

    // Tabify all docks on the same area into the
    const auto dockWidgets = findChildren<QDockWidget *>();
    for (auto dockWidget : dockWidgets) {
        if (dockWidget == dock)
            continue;
        if (dockWidgetArea(dockWidget) == area) {
            tabifyDockWidget(dockWidget, dock);
            return;
        }
    }
}
void MainWindow::followSymbol()
{
    auto *project = Core::Project::instance();
    auto *document = dynamic_cast<Core::LspDocument *>(project->currentDocument());

    if (document) {
        document->followSymbol();
    }
}

void MainWindow::switchDeclarationDefinition()
{
    auto *project = Core::Project::instance();
    auto *document = dynamic_cast<Core::LspDocument *>(project->currentDocument());

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

void MainWindow::runScript()
{
    RunScriptDialog dialog(this);
    dialog.exec();
}

void MainWindow::openOptions()
{
    OptionsDialog dialog(this);
    dialog.exec();
}

void MainWindow::showPalette()
{
    const int x = (width() - m_palette->width()) / 2;
    const int y = menuBar()->height() - 1;

    m_palette->move(mapToGlobal(QPoint {x, y}));
    m_palette->show();
    m_palette->raise();
}

void MainWindow::updateActions()
{
    auto document = Core::Project::instance()->currentDocument();

    ui->actionClose_Document->setEnabled(document != nullptr);

    auto *textDocument = qobject_cast<Core::TextDocument *>(document);
    ui->actionFind->setEnabled(textDocument != nullptr);
    ui->actionReplace->setEnabled(textDocument != nullptr);
    ui->actionFind_Next->setEnabled(textDocument != nullptr);
    ui->actionFind_Previous->setEnabled(textDocument != nullptr);

    auto *lspDocument = qobject_cast<Core::LspDocument *>(document);
    const bool lspEnabled = lspDocument && lspDocument->hasLspClient();
    ui->actionFollow_Symbol->setEnabled(lspEnabled);
    ui->actionSwitch_Decl_Def->setEnabled(lspEnabled);

    const bool cppEnabled = lspDocument && qobject_cast<Core::LspDocument *>(document);
    ui->actionSwitch_Header_Source->setEnabled(cppEnabled);

    const bool rcEnabled = qobject_cast<Core::RcDocument *>(document);
    ui->actionCreate_Qrc->setEnabled(rcEnabled);
    ui->actionCreate_Ui->setEnabled(rcEnabled);
}

void MainWindow::returnToEditor()
{
    if (QApplication::focusWidget() == ui->tabWidget->currentWidget())
        ui->findWidget->hide();
    else
        ui->tabWidget->currentWidget()->setFocus(Qt::FocusReason::ShortcutFocusReason);
}

void MainWindow::changeTab()
{
    if (ui->tabWidget->count() == 0) {
        ui->actionCreate_Qrc->setEnabled(false);
        ui->actionCreate_Ui->setEnabled(false);
        m_projectView->selectionModel()->clear();
        return;
    }

    auto document = Core::Project::instance()->open(ui->tabWidget->currentWidget()->windowTitle());
    if (!document)
        return;

    ui->actionCreate_Qrc->setEnabled(document->type() == Core::Document::Type::Rc);
    ui->actionCreate_Ui->setEnabled(document->type() == Core::Document::Type::Rc);
    ui->actionSwitch_Header_Source->setEnabled(document->type() == Core::Document::Type::Cpp);
}

static QWidget *widgetForDocument(Core::Document *document)
{
    switch (document->type()) {
    case Core::Document::Type::Cpp:
    case Core::Document::Type::Text: {
        auto textEdit = qobject_cast<Core::TextDocument *>(document)->textEdit();
        GuiSettings::setupDocumentTextEdit(textEdit, document->fileName());
        return textEdit;
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
