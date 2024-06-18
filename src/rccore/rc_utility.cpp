/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "rcfile.h"

namespace RcCore {

void RcFile::mergeLanguages(const QStringList &languages, const QString &newLanguage)
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
        newData.dialogDataList.append(d.dialogDataList);
        newData.dialogs.append(d.dialogs);
        newData.icons.append(d.icons);
        newData.menus.append(d.menus);
        newData.strings.insert(d.strings);
        newData.toolBars.append(d.toolBars);
        newData.ribbons.append(d.ribbons);
    }

    for (const auto &lang : languages)
        data.remove(lang);
    data[newLanguage] = newData;
}

} // namespace RcCore
