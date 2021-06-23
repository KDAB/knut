#pragma once

#include <QMainWindow>

class QMenu;

namespace Gui {

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void openProject();
    void initProject(const QString &path);
    void updateRecentProjects();

private:
    Ui::MainWindow *ui;
    QMenu *m_recentProjects = nullptr;
};

} // namespace Gui
