#pragma once

#include <QDialog>
#include <memory>

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
    ~RcToQrcDialog() override;
    void languageChanged();

public slots:
    void accept() override;

private:
    void updateColorBox();

private:
    std::unique_ptr<Ui::RcToQrcDialog> ui;
    Core::RcDocument *const m_document;
};

} // namespace Gui
