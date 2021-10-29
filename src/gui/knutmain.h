#pragma once

#include "core/knutcore.h"

namespace Gui {

class KnutMain : public Core::KnutCore
{
    Q_OBJECT
public:
    explicit KnutMain(QObject *parent = nullptr);

protected:
    void initParser(QCommandLineParser &parser) const override;
    void doParse(const QCommandLineParser &parser) const override;
};

} // namespace Gui
