#pragma once

#include <QWidget>

#include <memory>

namespace Gui {

namespace Ui {
    class Palette;
}

class PaletteFileModel;

class Palette : public QWidget
{
    Q_OBJECT

public:
    explicit Palette(QWidget *parent = nullptr);
    ~Palette();

    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void openDocument(const QModelIndex &index);

private:
    std::unique_ptr<Ui::Palette> ui;
    PaletteFileModel *m_fileModel;
};

} // namespace Gui
