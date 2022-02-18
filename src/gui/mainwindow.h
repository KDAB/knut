#pragma once

#include "interfacesettings.h"

#include <QMainWindow>

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
    void resizeEvent(QResizeEvent *event) override;

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
    void openSettings();
    void openInterfaceSettings();
    void showPalette();

    void changeTab();
    void changeCurrentDocument();
    void saveAllDocuments();

private:
    Ui::MainWindow *const ui;
    QMenu *m_recentProjects = nullptr;
    QFileSystemModel *m_fileModel = nullptr;
    Palette *m_palette = nullptr;
    InterfaceSettings m_settings;
};

} // namespace Gui
