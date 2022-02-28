#pragma once

#include "abstractpanel.h"

class QPlainTextEdit;
class QToolButton;

namespace Gui {

class LogPanel : public AbstractPanel
{
public:
    explicit LogPanel();
    ~LogPanel();

    QWidget *widget() const override;
    QWidget *toolBar() const override;
    QString title() const override;

private:
    QPlainTextEdit *const m_textEdit = nullptr;
    QWidget *const m_titleBar = nullptr;
};

} // namespace Gui
