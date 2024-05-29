#pragma once

#include <QDialog>
#include <memory>

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
    ~RcToUiDialog() override;
    void languageChanged();

public slots:
    void accept() override;

private:
    std::unique_ptr<Ui::RcToUiDialog> ui;
    Core::RcDocument *const m_document;
};

} // namespace Gui
