#include "textlocation.h"

#include "lspdocument.h"
#include "project.h"

namespace Core {

QString TextLocation::toString() const
{
    return QString("{'%1', %2}").arg(document->fileName()).arg(range.toString());
}

QVector<TextLocation> TextLocation::fromLsp(const std::vector<Lsp::Location> &locations)
{
    QVector<Core::TextLocation> textLocations;

    for (const auto &location : locations) {
        const auto url = QUrl::fromEncoded(QByteArray::fromStdString(location.uri));
        if (!url.isLocalFile()) {
            continue;
        }
        const auto filepath = url.toLocalFile();

        if (auto *document = qobject_cast<LspDocument *>(Project::instance()->get(filepath))) {
            const auto range = document->toRange(location.range);

            textLocations.emplace_back(TextLocation {.document = document, .range = range});
        }
    }

    return textLocations;
}

} // namespace Core
