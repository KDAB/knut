#pragma once

#include <QPlainTextEdit>

class QToolButton;

namespace Gui {

class LogPanel : public QPlainTextEdit
{
public:
    explicit LogPanel(QWidget *parent = nullptr);
    ~LogPanel();

    QWidget *toolBar() const;

private:
    QWidget *const m_toolBar = nullptr;
};

} // namespace Gui
