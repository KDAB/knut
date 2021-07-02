#include "cppdocument.h"

namespace Core {

CppDocument::CppDocument(QObject *parent)
    : TextDocument(Type::Cpp, parent)
{
}

} // namespace Core
