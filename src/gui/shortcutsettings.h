#pragma once

#include <QWidget>

namespace Gui {

namespace Ui {
    class ShortcutSettings;
}

class ShortcutModel;

class ShortcutSettings : public QWidget
{
    Q_OBJECT

public:
    explicit ShortcutSettings(QWidget *parent = nullptr);
    ~ShortcutSettings();

private:
    void updateCurrentItem();
    void resetAll();
    void resetSelected();
    void recordShortcut();

    Ui::ShortcutSettings *ui;
    ShortcutModel *const m_shortcutModel;
};

} // namespace Gui
