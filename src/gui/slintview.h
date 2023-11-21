#pragma once

#include "textview.h"

namespace Core {
class SlintDocument;
}

class QProcess;

namespace Gui {

class SlintView : public TextView
{
    Q_OBJECT

public:
    explicit SlintView(QWidget *parent = nullptr);
    ~SlintView() override;

private:
    void runSlint();

private:
    QProcess *m_process = nullptr;
};

}
