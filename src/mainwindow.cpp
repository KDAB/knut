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

#include <QApplication>
#include <QFileDialog>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->overviewTree->setResourceData(&m_data);
    ui->contentTree->setResourceData(&m_data);

    auto palette = ui->rcText->palette();
    palette.setColor(QPalette::Highlight, palette.color(QPalette::Highlight));
    palette.setColor(QPalette::HighlightedText, palette.color(QPalette::HighlightedText));
    ui->rcText->setPalette(palette);

    connect(ui->overviewTree, &OverviewTree::rcLineChanged, this, &MainWindow::highlightLine);
    connect(ui->contentTree, &ContentTree::rcLineChanged, this, &MainWindow::highlightLine);
    connect(ui->overviewTree, &OverviewTree::dataSelected, ui->contentTree, &ContentTree::setData);

    connect(ui->actionExit, &QAction::triggered, QApplication::instance(), &QApplication::quit);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openData);
    connect(ui->actionExtractActions, &QAction::triggered, this, &MainWindow::extractActions);
    connect(ui->actionExtractMenus, &QAction::triggered, this, &MainWindow::extractMenus);

    new RcSyntaxHighlighter(ui->rcText->document());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::highlightLine(int line)
{
    if (line == -1) {
        ui->rcText->setTextCursor({});
        return;
    }

    QTextCursor cursor(ui->rcText->document()->findBlockByLineNumber(line - 1));
    cursor.select(QTextCursor::LineUnderCursor);
    ui->rcText->setTextCursor(cursor);
}

void MainWindow::openData()
{
    const QString &fileName = QFileDialog::getOpenFileName(this, "Open Resource File", ".", "*.rc");
    if (fileName.isEmpty())
        return;

    ui->contentTree->clear();

    m_data = Parser::parse(fileName);

    setWindowTitle(QString("Knut - %1").arg(fileName));
    ui->rcText->setPlainText(m_data.content.replace("\t", "    "));
    ui->overviewTree->updateModel();
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
