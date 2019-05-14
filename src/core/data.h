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
        int line;
        QString fileName;
        bool exist;
    };
    struct Asset
    {
        int line;
        QString id;
        QString fileName;
        bool exist;
    };
    struct String
    {
        int line;
        QString id;
        QString text;
    };
    struct DialogData
    {
        int line;
        QString id;
        QHash<QString, QStringList> values;
    };
    struct Accelerator
    {
        int line;
        QString id;
        QString shortcut;

        bool isUnknown() const { return shortcut.contains("VK_"); }
    };
    struct AcceleratorTable
    {
        int line;
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
        int line;
        QString id;
        QString text;
        QString shortcut;
        int flags = 0;
        QVector<MenuItem> children;
    };
    struct ToolBarItem
    {
        int line;
        QString id;
    };
    struct ToolBar
    {
        int line;
        QString id;
        int width = 0;
        int height = 0;
        QVector<ToolBarItem> children;
    };
    struct Control
    {
        int line;
        QString type;
        QString text;
        QString id;
        QString className;
        QRect geometry;
        QStringList styles;
    };
    struct Dialog
    {
        int line;
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
