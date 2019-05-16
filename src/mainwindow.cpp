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
#include <QShortcut>
#include <QPushButton>
#include <QTextEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->overviewTree->setResourceData(&m_data);
    ui->contentTree->setResourceData(&m_data);

    auto palette = ui->texteditwidget->textEdit()->palette();
    palette.setColor(QPalette::Highlight, palette.color(QPalette::Highlight));
    palette.setColor(QPalette::HighlightedText, palette.color(QPalette::HighlightedText));
    ui->texteditwidget->textEdit()->setPalette(palette);

    connect(ui->overviewTree, &OverviewTree::rcLineChanged, this, &MainWindow::highlightLine);
    connect(ui->contentTree, &ContentTree::rcLineChanged, this, &MainWindow::highlightLine);
    connect(ui->overviewTree, &OverviewTree::dataSelected, ui->contentTree, &ContentTree::setData);

    connect(ui->actionClose, &QAction::triggered, this, &MainWindow::closeFile);
    connect(ui->actionExit, &QAction::triggered, QApplication::instance(), &QApplication::quit);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openData);
    connect(ui->actionExtractActions, &QAction::triggered, this, &MainWindow::extractActions);
    connect(ui->actionExtractMenus, &QAction::triggered, this, &MainWindow::extractMenus);

    new RcSyntaxHighlighter(ui->texteditwidget->textEdit()->document());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeFile()
{
    m_data = {};
    ui->contentTree->clear();
    ui->texteditwidget->textEdit()->clear();
    ui->overviewTree->updateModel();
}

void MainWindow::highlightLine(int line)
{
    if (line == -1) {
        ui->texteditwidget->textEdit()->setTextCursor({});
        return;
    }

    QTextCursor cursor(ui->texteditwidget->textEdit()->document()->findBlockByLineNumber(line - 1));
    cursor.select(QTextCursor::LineUnderCursor);
    ui->texteditwidget->textEdit()->setTextCursor(cursor);
}

void MainWindow::openData()
{
    const QString &fileName = QFileDialog::getOpenFileName(this, QStringLiteral("Open Resource File"), QStringLiteral("."), QStringLiteral("*.rc"));
    if (fileName.isEmpty())
        return;

    ui->contentTree->clear();

    m_data = Parser::parse(fileName);

    setWindowTitle(QStringLiteral("Knut - %1").arg(fileName));
    ui->texteditwidget->textEdit()->setPlainText(m_data.content.replace(QLatin1Char('\t'), QLatin1String("    ")));
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
