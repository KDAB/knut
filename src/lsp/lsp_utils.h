#pragma once

#include "types.h"

#include <QUrl>

namespace Lsp {
namespace Utils {

    inline DocumentUri toDocumentUri(QString localFile)
    {
        return QUrl::fromLocalFile(localFile).toString().toStdString();
    }

}
}
