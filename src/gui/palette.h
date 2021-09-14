#pragma once

#include <QWidget>

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
    Ui::Palette *ui;
    PaletteFileModel *m_fileModel;
};

} // namespace Gui
