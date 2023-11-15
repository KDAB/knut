#pragma once

#include <QPlainTextEdit>

namespace Gui {

class LogPanel : public QPlainTextEdit
{
public:
    explicit LogPanel(QWidget *parent = nullptr);
    ~LogPanel() override;

    QWidget *toolBar() const;

private:
    QWidget *const m_toolBar = nullptr;
};

} // namespace Gui
