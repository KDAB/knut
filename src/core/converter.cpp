#include "converter.h"

#include "data.h"
#include "logging.h"

#include <QFileInfo>
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
    if (!menu.children.empty()) {
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
    }
    return QVariant::fromValue(Shortcut {accelerator.shortcut});
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

MenuItem createMenuItem(const Data::MenuItem &item)
{
    MenuItem menu;
    menu.id = item.id;
    menu.title = item.text;
    if (item.id.isEmpty() && item.text.isEmpty())
        menu.isSeparator = true;
    else if (item.children.isEmpty())
        menu.isAction = true;

    for (const auto &child : item.children)
        menu.children.push_back(QVariant::fromValue(createMenuItem(child)));

    return menu;
}

QVariantList convertMenus(Data *data, const Knut::DataCollection &collection)
{
    QVariantList result;
    for (const auto &item : collection) {
        const int index = item.second;

        const auto menuBar = data->menus.value(index);
        Menu menu;
        menu.id = menuBar.id;

        QHash<QString, int> actionIdMap;
        for (const auto &childMenu : menuBar.children) {
            auto topMenu = createMenuItem(childMenu);
            topMenu.isTopLevel = true;
            createActionForMenu(data, topMenu.actions, actionIdMap, childMenu);
            menu.children.push_back(QVariant::fromValue(topMenu));
        }
        result.push_back(QVariant::fromValue(menu));
    }
    return result;
}

bool isAnActionFromMenu(const Data::MenuItem &submenu, const QString &actionId)
{
    if (submenu.id == actionId) {
        return true;
    }
    for (const auto &menu : submenu.children) {
        if (menu.id == actionId) {
            return true;
        }
        if (isAnActionFromMenu(menu, actionId)) {
            return true;
        }
    }
    return false;
}

ToolBarItem createToolBarItem(Data *data, const Data::ToolBarItem &child, ToolBar &itemToolbar, int index)
{
    ToolBarItem item;
    if (child.id.isEmpty()) {
        item.isSeparator = true;
    } else {
        item.id = child.id;

        bool foundAction = false;
        for (const auto &menu : data->menus) {
            if (isAnActionFromMenu(menu, item.id)) {
                foundAction = true;
                break;
            }
        }
        if (!foundAction) {
            const QString text = data->strings.value(child.id).text;
            if (!text.isEmpty()) {

                const auto tips = text.split(QLatin1Char('\n'));
                item.statusTip = tips.first();
                if (tips.size() > 1)
                    item.toolTip = tips.value(1);
            }
            itemToolbar.needToGenerateActions = true;
            item.iconName = itemToolbar.iconName + QLatin1Char('/') + itemToolbar.iconName + QStringLiteral("%1.png").arg(index);
        }
    }
    return item;
}

ToolBar convertToolbar(Data *data, const Knut::DataCollection &collection)
{
    Q_ASSERT(collection.size() == 1);
    const int index = collection.first().second;
    Data::ToolBar item = data->toolBars.value(index);
    ToolBar toolbar;
    toolbar.id = item.id;
    toolbar.iconSize = QSize(item.width, item.height);

    const QFileInfo fileInfo(data->assets.value(toolbar.id).fileName);
    QString filename = fileInfo.fileName();
    filename.remove(QStringLiteral(".bmp"));
    toolbar.iconName = filename;

    int indexIcon = 0;
    for (const auto &child : item.children) {
        toolbar.children.push_back(QVariant::fromValue(createToolBarItem(data, child, toolbar, indexIcon)));
        if (!child.id.isEmpty()) {
            indexIcon++;
        }
    }

    return toolbar;
}

QVector<Asset> convertAssets(Data *data, const QDir &relativeDir)
{
    QVector<Asset> result;
    for (const auto &item : data->assets) {
        Asset asset;
        asset.id = item.id;
        if (!relativeDir.isEmpty())
            asset.fileName = relativeDir.relativeFilePath(item.fileName);
        else
            asset.fileName = item.fileName;
        result.push_back(asset);
    }
    return result;
}
}
