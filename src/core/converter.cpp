#include "converter.h"

#include "data.h"
#include "logging.h"

#include <QHash>

namespace Converter {

static void fillTips(Data *data, const QString &id, Action &action)
{
    const auto &text = data->strings.value(id);
    if (!text.text.isEmpty()) {
        const auto tips = text.text.split(QLatin1Char('\n'));
        action.statusTip = tips.first();
        if (tips.size() > 1)
            action.toolTip = tips.value(1);
    }
}

static void createActionForMenu(Data *data, QVariantList &actions, QHash<QString, int> &actionIdMap,
                                const Data::MenuItem &menu)
{
    if (menu.children.size()) {
        for (const auto &child : menu.children)
            createActionForMenu(data, actions, actionIdMap, child);
    } else if (!menu.id.isEmpty()) {
        // We stop here in case of duplication in the menu
        if (actionIdMap.contains(menu.id)) {
            qCWarning(CONVERTER) << "Duplicate action in menu:" << menu.id;
            return;
        }
        Action action;
        action.id = menu.id;
        action.title = menu.text;
        action.checked = menu.flags & Data::MenuItem::Checked;
        if (!menu.shortcut.isEmpty())
            action.shortcuts.push_back(QVariant::fromValue(Shortcut {menu.shortcut}));
        fillTips(data, menu.id, action);
        actionIdMap[menu.id] = actions.size();
        actions.push_back(QVariant::fromValue(action));
    }
}

static QVariant createShortcut(const Data::Accelerator &accelerator)
{
    if (accelerator.isUnknown()) {
        qCWarning(CONVERTER) << "Unknown shortcut:" << accelerator.id;
        return QVariant::fromValue(Shortcut {accelerator.shortcut, true});
    } else {
        return QVariant::fromValue(Shortcut {accelerator.shortcut});
    }
}

static void createActionForAccelerator(Data *data, QVariantList &actions,
                                       QHash<QString, int> &actionIdMap,
                                       const Data::AcceleratorTable &item)
{
    for (const auto &accelerator : item.accelerators) {

        const int index = actionIdMap.value(accelerator.id, -1);
        if (index != -1) {
            auto action = actions.value(index).value<Action>();
            action.shortcuts.push_back(createShortcut(accelerator));
            actions[index] = QVariant::fromValue(action);
        } else {
            Action action;
            action.id = accelerator.id;
            action.shortcuts.push_back(createShortcut(accelerator));
            fillTips(data, accelerator.id, action);
        }
    }
}

QVariantList convertActions(Data *data, const Knut::DataCollection &collection)
{
    QVariantList actions;
    QHash<QString, int> actionIdMap;

    for (const auto &item : collection) {
        if (item.first == Knut::MenuData) {
            createActionForMenu(data, actions, actionIdMap, data->menus.value(item.second));
        } else {
            createActionForAccelerator(data, actions, actionIdMap,
                                       data->acceleratorTables.value(item.second));
        }
    }
    return actions;
}

Menu createMenu(const Data::MenuItem &item)
{
    Menu menu;
    menu.id = item.id;
    menu.title = item.text;
    if (item.id.isEmpty() && item.text.isEmpty())
        menu.isSeparator = true;
    else if (item.children.isEmpty())
        menu.isAction = true;

    for (const auto &child : item.children)
        menu.children.push_back(QVariant::fromValue(createMenu(child)));

    return menu;
}

Menu convertMenus(Data *data, const Knut::DataCollection &collection)
{
    Q_ASSERT(collection.size() == 1);
    const int index = collection.first().second;
    return createMenu(data->menus.value(index));
}

}
