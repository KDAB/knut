#pragma once

#include "textview.h"

namespace Core {
class QmlDocument;
}

namespace Gui {

class QmlView : public TextView
{
    Q_OBJECT
public:
    explicit QmlView(QWidget *parent = nullptr);
    ~QmlView() override;

private:
    void runQml();
};

} // namespace Gui
