#include "rcfile.h"

namespace RcCore {

void RcFile::mergeLanguages(const QStringList languages, const QString &newLanguage)
{
    if (languages.isEmpty() || (languages.count() == 1 && languages.first() == newLanguage))
        return;

    Data newData = data.value(newLanguage, {});
    newData.language = newLanguage;
    newData.fileName = fileName;

    for (const auto &d : std::as_const(data)) {
        if (!languages.contains(d.language))
            continue;
        newData.acceleratorTables.append(d.acceleratorTables);
        newData.assets.append(d.assets);
        newData.dialogData.append(d.dialogData);
        newData.dialogs.append(d.dialogs);
        newData.icons.append(d.icons);
        newData.menus.append(d.menus);
        newData.strings.insert(d.strings);
        newData.toolBars.append(d.toolBars);
    }

    for (const auto &lang : languages)
        data.remove(lang);
    data[newLanguage] = newData;
}

} // namespace RcCore
