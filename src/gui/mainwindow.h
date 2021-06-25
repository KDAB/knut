#pragma once

#include <QHash>
#include <QMainWindow>

class QMenu;
class QFileSystemModel;

namespace Core {
class TextDocument;
class RcDocument;
}

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
    void openDocument(const QModelIndex &index);
    void createQrc();
    void createUi();
    void runScript();

    void changeTab();
    void changeCurrentDocument();

private:
    Ui::MainWindow *ui;
    QMenu *m_recentProjects = nullptr;
    QHash<QString, int> m_windows;
    QFileSystemModel *m_fileModel = nullptr;
};

} // namespace Gui
