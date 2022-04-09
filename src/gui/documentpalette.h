#pragma once

#include <QListWidget>

namespace Gui {

class DocumentPalette : public QListWidget
{
    Q_OBJECT
public:
    explicit DocumentPalette(QWidget *parent = nullptr);

    void showWindow();

protected:
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    void fillList();
    void updateListHeight();
    void changeCurrentDocument(int index);
};

} // namespace Gui
