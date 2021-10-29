#pragma once

#include <QDialog>

namespace Core {
class RcDocument;
}

namespace Gui {

namespace Ui {
    class RcToQrcDialog;
}

class RcToQrcDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RcToQrcDialog(Core::RcDocument *document, QWidget *parent = nullptr);
    ~RcToQrcDialog();

public slots:
    void accept() override;

private:
    void updateColorBox();

private:
    Ui::RcToQrcDialog *const ui;
    Core::RcDocument *const m_document;
};

} // namespace Gui
