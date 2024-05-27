#include "imagedocument.h"

#include "utils/log.h"

namespace Core {

ImageDocument::ImageDocument(QObject *parent)
    : Document(Type::Image, parent)
{
}

QImage ImageDocument::image() const
{
    return m_image;
}

bool ImageDocument::doSave(const QString &fileName)
{
    Q_UNUSED(fileName)
    spdlog::error("ImageDocument::doSave - not implemented yet");
    return false;
}

bool ImageDocument::doLoad(const QString &fileName)
{
    return m_image.load(fileName);
}

} // namespace Core
