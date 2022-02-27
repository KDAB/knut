#pragma once

#include <QPlainTextEdit>

namespace Gui {

class LogPanel : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit LogPanel(QWidget *parent = nullptr);
    ~LogPanel();
};

} // namespace Gui
