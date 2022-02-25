#pragma once

#include <QPlainTextEdit>

namespace Gui {

class LogViewer : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit LogViewer(QWidget *parent = nullptr);
    ~LogViewer();
};

} // namespace Gui
