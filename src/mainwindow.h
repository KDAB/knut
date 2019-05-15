#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "data.h"
#include "global.h"

#include <QMainWindow>

class OverviewModel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void highlightLine(int line);

private:
    void openData();
    void extractActions();
    void extractMenus();
    void closeFile();

private:
    Ui::MainWindow *ui;
    Data m_data;
};

#endif // MAINWINDOW_H
