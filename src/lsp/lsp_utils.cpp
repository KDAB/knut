#include "lsp_utils.h"

namespace Lsp::Utils {

QString removeTypeAliasInformation(const QString &typeInfo)
{

    auto result = typeInfo;
    qsizetype index;
    while ((index = result.indexOf("(aka ")) != -1) {
        qsizetype start = index;
        int braceCount = 0;
        // find the correct matching ")"
        for (; index < result.size() && braceCount >= 0; index++) {
            if (result[index] == '(') {
                braceCount++;
            } else if (result[index] == ')') {
                braceCount--;
            }
        }
        qsizetype end = index;
        result.erase(result.cbegin() + start, result.cbegin() + end);
    }
    return result.trimmed();
}

}
