#ifndef GUI_TRANSFORMPREVIEWDIALOG_H
#define GUI_TRANSFORMPREVIEWDIALOG_H

#include "core/codedocument.h"

#include <QDialog>

namespace Gui {

namespace Ui {
    class TransformPreviewDialog;
}

class TransformPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransformPreviewDialog(Core::CodeDocument *document, const QString &resultText, int numberReplacements,
                                    QWidget *parent = nullptr);
    ~TransformPreviewDialog() override;

private:
    Ui::TransformPreviewDialog *ui;
};

} // namespace Gui
#endif // GUI_TRANSFORMPREVIEWDIALOG_H
