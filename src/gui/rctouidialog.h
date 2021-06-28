#ifndef GUI_RCTOUIDIALOG_H
#define GUI_RCTOUIDIALOG_H

#include <QDialog>

namespace Core {
class RcDocument;
}

namespace Gui {

namespace Ui {
    class RcToUiDialog;
}

class RcToUiDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RcToUiDialog(Core::RcDocument *document, QWidget *parent = nullptr);
    ~RcToUiDialog();

public slots:
    void accept() override;

private:
    Ui::RcToUiDialog *const ui;
    Core::RcDocument *const m_document;
};

} // namespace Gui
#endif // GUI_RCTOUIDIALOG_H
