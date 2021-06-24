#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "rctoqrcdialog.h"
#include "rctouidialog.h"

#include "core/document.h"
#include "core/project.h"
#include "core/rcdocument.h"
#include "core/textdocument.h"
#include "rcui/rcfileview.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <QSettings>

namespace Gui {

constexpr int MaximumRecentProjects = 10;
constexpr char RecentProjectKey[] = "RecentProject";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    ui->splitter->setSizes({200, 600});
    setWindowTitle(QApplication::applicationName() + ' ' + QApplication::applicationVersion());

    connect(ui->action_Quit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->action_Open, &QAction::triggered, this, &MainWindow::openProject);
    connect(ui->actionCreate_Qrc, &QAction::triggered, this, &MainWindow::createQrc);
    connect(ui->actionCreate_Ui, &QAction::triggered, this, &MainWindow::createUi);

    m_recentProjects = new QMenu(this);
    ui->actionRecent_Projects->setMenu(m_recentProjects);
    updateRecentProjects();

    auto path = Core::Project::instance()->root();
    if (!path.isEmpty())
        initProject(path);

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::changeCurrentDocument);
}

MainWindow::~MainWindow()
{
    delete ui;
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
    auto projectModel = new QFileSystemModel(this);
    auto index = projectModel->setRootPath(path);
    ui->projectView->setModel(projectModel);
    for (int i = 1; i < projectModel->columnCount(); ++i)
        ui->projectView->header()->hideSection(i);
    ui->projectView->setRootIndex(index);
    connect(ui->projectView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::openDocument);

    // Disable menus, we can only load one project - restart Knut if needed
    ui->action_Open->setEnabled(false);
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
    auto fileSystemModel = qobject_cast<QFileSystemModel *>(ui->projectView->model());
    auto path = fileSystemModel->filePath(index);
    auto document = Core::Project::instance()->open(path);
    if (!document)
        return;

    // open the window if it's already opened
    int windowIndex = m_windows.value(document->fileName(), -1);
    if (windowIndex != -1) {
        ui->tabWidget->setCurrentIndex(windowIndex);
        return;
    }

    QDir dir(Core::Project::instance()->root());
    windowIndex = ui->tabWidget->addTab(document->widget(), dir.relativeFilePath(document->fileName()));
    m_windows[document->fileName()] = windowIndex;
    ui->tabWidget->setCurrentIndex(windowIndex);
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

void MainWindow::changeCurrentDocument(int index)
{
    auto document = Core::Project::instance()->open(ui->tabWidget->tabText(index));
    if (!document)
        return;

    ui->actionCreate_Qrc->setEnabled(document->type() == Core::Document::Type::Rc);
    ui->actionCreate_Ui->setEnabled(document->type() == Core::Document::Type::Rc);
}

} // namespace Gui
