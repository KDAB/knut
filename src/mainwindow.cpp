#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "actiondialog.h"
#include "data.h"
#include "global.h"
#include "menudialog.h"
#include "menumodel.h"
#include "overviewmodel.h"
#include "parser.h"
#include "rcsyntaxhighlighter.h"
#include "toolbardialog.h"
#include "widgetdialog.h"

#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QMenu>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>

namespace {
constexpr int MaximumRecentFile = 5;
constexpr char RecentFileKey[] = "recentFileList";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->overviewTree->setResourceData(&m_data);
    ui->contentTree->setResourceData(&m_data);

    connect(ui->overviewTree, &OverviewTree::rcLineChanged, this, &MainWindow::highlightLine);
    connect(ui->contentTree, &ContentTree::rcLineChanged, this, &MainWindow::highlightLine);
    connect(ui->overviewTree, &OverviewTree::dataSelected, ui->contentTree, &ContentTree::setData);

    connect(ui->actionClose, &QAction::triggered, this, &MainWindow::closeFile);
    connect(ui->actionExit, &QAction::triggered, QApplication::instance(), &QApplication::quit);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openData);
    connect(ui->actionExtractActions, &QAction::triggered, this, &MainWindow::extractActions);
    connect(ui->actionExtractMenus, &QAction::triggered, this, &MainWindow::extractMenus);
    connect(ui->actionExtractToolBars, &QAction::triggered, this, &MainWindow::extractToolbars);
    connect(ui->actionExtractWidgets, &QAction::triggered, this, &MainWindow::extractWidgets);

    new RcSyntaxHighlighter(ui->texteditwidget->document());
    m_recentMenu = new QMenu(this);
    ui->actionOpenRecent->setMenu(m_recentMenu);
    updateRecentFileActions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeFile()
{
    m_data = {};
    ui->contentTree->clear();
    ui->texteditwidget->clear();
    ui->overviewTree->updateModel();
}

void MainWindow::highlightLine(int line)
{
    if (line == -1) {
        ui->texteditwidget->setTextCursor({});
        return;
    }

    QTextCursor cursor(ui->texteditwidget->document()->findBlockByLineNumber(line - 1));
    cursor.select(QTextCursor::LineUnderCursor);
    ui->texteditwidget->setTextCursor(cursor);
}

void MainWindow::openData()
{
    const QString &fileName = QFileDialog::getOpenFileName(
        this, QStringLiteral("Open Resource File"), QStringLiteral("."), QStringLiteral("*.rc"));
    if (fileName.isEmpty())
        return;
    openFile(fileName);
    updateRecentFiles(fileName);
}

void MainWindow::updateRecentFiles(const QString &fileName)
{
    QSettings settings;
    QStringList files = settings.value(QLatin1String(RecentFileKey)).toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaximumRecentFile)
        files.removeLast();

    settings.setValue(QLatin1String(RecentFileKey), files);
    updateRecentFileActions();
}

void MainWindow::extractActions()
{
    ActionDialog dialog(&m_data, this);
    dialog.exec();
}

void MainWindow::extractMenus()
{
    MenuDialog dialog(&m_data, this);
    dialog.exec();
}

void MainWindow::openFile(const QString &fileName)
{
    ui->contentTree->clear();

    m_data = Parser::parse(fileName);

    setWindowTitle(QStringLiteral("Knut - %1").arg(fileName));
    ui->texteditwidget->setText(m_data.content.replace(QLatin1Char('\t'), QLatin1String("    ")));
    ui->overviewTree->updateModel();
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    const QStringList files = settings.value(QLatin1String(RecentFileKey)).toStringList();

    const int numRecentFiles = qMin(files.count(), MaximumRecentFile);
    m_recentMenu->clear();
    for (int i = 0; i < numRecentFiles; ++i) {
        const QString text = files[i];
        QAction *act = m_recentMenu->addAction(text);
        connect(act, &QAction::triggered, this, [this, text]() { openFile(text); });
    }
    ui->actionOpenRecent->setEnabled(MaximumRecentFile > 0);
}

void MainWindow::extractToolbars()
{
    ToolbarDialog dialog(&m_data, this);
    dialog.exec();
}

void MainWindow::extractWidgets()
{
    WidgetDialog dialog(&m_data, this);
    dialog.exec();
}