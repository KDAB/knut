#pragma once

#include <QFrame>
#include <QToolBar>

namespace Gui {

class Toolbar : public QToolBar
{
    Q_OBJECT

public:
    explicit Toolbar(QWidget *parent = nullptr);

private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void relayout();
};

}
