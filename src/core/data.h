#ifndef DATA_H
#define DATA_H

#include <QByteArray>
#include <QHash>
#include <QRect>
#include <QString>
#include <QVector>

struct Data
{
    struct Include
    {
        int line = -1;
        QString fileName;
        bool exist = false;
    };
    struct Asset
    {
        int line = -1;
        QString id;
        QString fileName;
        bool exist = false;
    };
    struct String
    {
        int line = -1;
        QString id;
        QString text;
    };
    struct DialogData
    {
        int line = -1;
        QString id;
        QHash<QString, QStringList> values;
    };
    struct Accelerator
    {
        int line = -1;
        QString id;
        QString shortcut;

        bool isUnknown() const { return shortcut.contains(QLatin1String("VK_")); }
    };
    struct AcceleratorTable
    {
        int line = -1;
        QString id;
        QVector<Accelerator> accelerators;
    };
    struct MenuItem
    {
        enum Flags {
            Checked = 0x01,
            Grayed = 0x02,
            Help = 0x04,
            Inactive = 0x08,
            MenuBarBreak = 0x10,
            MenuBreak = 0x20,
        };
        int line = -1;
        QString id;
        QString text;
        QString shortcut;
        int flags = 0;
        QVector<MenuItem> children;
    };
    struct ToolBarItem
    {
        int line = -1;
        QString id;
    };
    struct ToolBar
    {
        int line = -1;
        QString id;
        int width = 0;
        int height = 0;
        QVector<ToolBarItem> children;
    };
    struct Control
    {
        int line = -1;
        QString type;
        QString text;
        QString id;
        QString className;
        QRect geometry;
        QStringList styles;
    };
    struct Dialog
    {
        int line = -1;
        QString id;
        QRect geometry;
        QString caption;
        QString menu;
        QStringList styles;
        QVector<Control> controls;
    };

    QString fileName;
    QString content;
    bool isValid = false;

    // Resources
    QVector<Include> includes;
    QHash<QString, Asset> icons;
    QHash<QString, Asset> assets;
    QHash<QString, String> strings;
    QVector<AcceleratorTable> acceleratorTables;
    QVector<MenuItem> menus;
    QVector<ToolBar> toolBars;
    QVector<DialogData> dialogData;
    QVector<Dialog> dialogs;

    QHash<int, QString> resourceMap;
};

#endif // DATA_H
