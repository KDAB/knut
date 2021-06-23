#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "core/project.h"

#include <QApplication>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QHeaderView>
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

    m_recentProjects = new QMenu(this);
    ui->actionRecent_Projects->setMenu(m_recentProjects);
    updateRecentProjects();

    auto path = Core::Project::instance()->root();
    if (!path.isEmpty())
        initProject(path);
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
        const QString text = projects.value(i);
        QAction *act = m_recentProjects->addAction(text);
        connect(act, &QAction::triggered, this, [this, text]() {
            initProject(text);
        });
    }
    ui->actionRecent_Projects->setEnabled(numRecentProjects > 0);
}

} // namespace Gui
