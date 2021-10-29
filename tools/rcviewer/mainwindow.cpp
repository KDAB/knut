#include "mainwindow.h"

#include "rccore/rcfile.h"
#include "rcui/rcfileview.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_view = new RcUi::RcFileView(this);
    setCentralWidget(m_view);

    auto fileMenu = menuBar()->addMenu(tr("&File"));
    auto loadAction = fileMenu->addAction(tr("&Load"));
    connect(loadAction, &QAction::triggered, this, &MainWindow::load);
    fileMenu->addSeparator();
    auto quitAction = fileMenu->addAction(tr("&Quit"));
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    resize(1024, 768);
}

void MainWindow::load()
{
    QFileInfo fi(m_fileName);

    auto fileName = QFileDialog::getOpenFileName(this, "RcViewer", fi.absolutePath(), "*.rc");
    if (fileName.isEmpty())
        return;

    m_fileName = fileName;
    m_data = RcCore::parse(m_fileName);
    m_view->setRcFile(m_data);
}
