#pragma once

#include <QString>

#include <memory>

class QWidget;

namespace Gui {

class AbstractPanel
{
public:
    virtual ~AbstractPanel() = default;

    virtual QWidget *widget() const = 0;
    virtual QWidget *toolBar() const = 0;
    virtual QString title() const = 0;
};

template <typename T>
std::unique_ptr<AbstractPanel> make_panel()
{
    return std::unique_ptr<AbstractPanel>(new T);
}

} // namespace Gui
