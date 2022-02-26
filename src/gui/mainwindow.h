#pragma once

#include <QMainWindow>

#include <memory>

class QMenu;
class QFileSystemModel;

namespace Core {
class TextDocument;
class RcDocument;
}

namespace Gui {

class Palette;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void openProject();
    void initProject(const QString &path);
    void updateRecentProjects();
    void openDocument(const QModelIndex &index);
    void saveDocument();
    void closeDocument();
    void createQrc();
    void createUi();
    void runScript();
    void openOptions();
    void showPalette();

    void changeTab();
    void changeCurrentDocument();
    void saveAllDocuments();

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QMenu *m_recentProjects = nullptr;
    QFileSystemModel *m_fileModel = nullptr;
    Palette *m_palette = nullptr;
};

} // namespace Gui
