#pragma once

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
