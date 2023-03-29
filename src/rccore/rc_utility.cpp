#include "rcfile.h"

namespace RcCore {

void mergeAllLanguages(RcFile &rcFile, const QString &newLanguage)
{
    Data newData;
    newData.fileName = rcFile.fileName;
    newData.language = newLanguage;

    for (const auto &d : std::as_const(rcFile.data)) {
        newData.acceleratorTables.append(d.acceleratorTables);
        newData.assets.append(d.assets);
        newData.dialogData.append(d.dialogData);
        newData.dialogs.append(d.dialogs);
        newData.icons.append(d.icons);
        newData.menus.append(d.menus);
        newData.strings.insert(d.strings);
        newData.toolBars.append(d.toolBars);
    }

    rcFile.data = {{newLanguage, newData}};
}

} // namespace RcCore
