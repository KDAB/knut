#pragma once

#include "document.h"

#include <QImage>

namespace Core {

class ImageDocument : public Document
{
    Q_OBJECT

public:
    ImageDocument(QObject *parent = nullptr);

    QImage image() const;

protected:
    bool doSave(const QString &fileName) override;
    bool doLoad(const QString &fileName) override;

private:
    QImage m_image;
};

} // namespace Core
