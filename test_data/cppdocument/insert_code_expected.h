#pragma once

#include <QMainWindow>

#include <memory>

namespace Gui {

class Palette;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void updateActions();
    void initProject(const QString &path);
    void openDocument(const QModelIndex &index);

private:
    QFileSystemModel *const m_fileModel = nullptr;
    QTreeView *const m_projectView = nullptr;
    Palette *const m_palette = nullptr;
};

} // namespace Gui
