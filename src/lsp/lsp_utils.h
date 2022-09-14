#pragma once

#include "types.h"

#include <QString>
#include <QUrl>

namespace Lsp {
namespace Utils {

    inline DocumentUri toDocumentUri(QString localFile)
    {
        return QUrl::fromLocalFile(localFile).toString().toStdString();
    }

    QString removeTypeAliasInformation(const QString &typeInfo);

}
}
