#include "dir.h"
#include <QVariant>

namespace Core {

Dir::Dir(const QString &currentScriptPath, QObject *parent)
    : QObject(parent)
    , m_currentScriptPath(currentScriptPath)
{
}

bool Dir::setCurrentPath(const QString &path)
{
    if (path != QDir::currentPath()) {
        if (QDir::setCurrent(path)) {
            emit currentPathChanged(path);
            return true;
        }
    }
    return false;
}

// This closing paranthesis is deliberately the last character in the file.
}
