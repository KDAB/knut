#pragma once

#include <QByteArray>
#include <QHash>
#include <QRect>
#include <QString>
#include <QVariant>
#include <QVector>

namespace RcFile {

struct Asset
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString fileName MEMBER fileName)
    Q_PROPERTY(bool exist MEMBER exist)
public:
    enum ConversionFlag {
        NoFlags = 0x0,
        RemoveUnknown = 0x01,
        SplitToolBar = 0x02,
        ConvertToPng = 0x04,
        AllFlags = RemoveUnknown | SplitToolBar | ConvertToPng,
    };
    Q_DECLARE_FLAGS(ConversionFlags, ConversionFlag)
    Q_FLAG(ConversionFlag)

    enum TransparentColor {
        NoColors = 0x0,
        Gray = 0x01,
        Magenta = 0x02,
        BottomLeftPixel = 0x04,
        AllColors = Gray | Magenta | BottomLeftPixel,
    };
    Q_DECLARE_FLAGS(TransparentColors, TransparentColor)
    Q_FLAG(TransparentColor)

    QString id;
    QString fileName;
    bool exist;

    // Internal data
    int line = -1;
    QString originalFileName;
    QRect iconRect;

    bool isSame() const { return originalFileName.isEmpty(); }
};

struct ToolBarItem
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(bool isSeparator READ isSeparator)
public:
    QString id;
    bool isSeparator() const { return id.isEmpty(); }
    // Internal data
    int line = -1;
};
bool operator==(const ToolBarItem &left, const ToolBarItem &right);

struct ToolBar
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QVector<RcFile::ToolBarItem> children MEMBER children)
    Q_PROPERTY(QSize iconSize MEMBER iconSize)
public:
    QString id;
    QSize iconSize;
    QVector<ToolBarItem> children;
    // Internal data
    int line = -1;
};

struct Widget
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString className MEMBER className)
    Q_PROPERTY(QRect geometry MEMBER geometry)
    Q_PROPERTY(QVariantMap properties MEMBER properties)
    Q_PROPERTY(QVector<RcFile::Widget> children MEMBER children)
public:
    enum ConversionFlag {
        NoFlags = 0x0,
        UpdateHierarchy = 0x01,
        UpdateGeometry = 0x02,
        AllFlags = UpdateHierarchy | UpdateGeometry,
    };
    Q_DECLARE_FLAGS(ConversionFlags, ConversionFlag)
    Q_FLAG(ConversionFlag)

    QString id;
    QString className;
    QRect geometry;
    QVariantMap properties;
    QVector<Widget> children;
};
bool operator==(const Widget &left, const Widget &right);

struct MenuItem
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString text MEMBER text)
    Q_PROPERTY(QVector<RcFile::MenuItem> children MEMBER children)
    Q_PROPERTY(bool isSeparator READ isSeparator)
    Q_PROPERTY(bool isAction READ isAction)
    Q_PROPERTY(bool isTopLevel MEMBER isTopLevel)
public:
    enum Flags {
        Checked = 0x01,
        Grayed = 0x02,
        Help = 0x04,
        Inactive = 0x08,
        MenuBarBreak = 0x10,
        MenuBreak = 0x20,
    };
    QString id;
    QString text;
    QVector<MenuItem> children;
    bool isSeparator() const { return id.isEmpty() && text.isEmpty(); }
    bool isAction() const { return !isSeparator() && children.isEmpty(); }
    bool isTopLevel = false;
    // Internal data
    QString shortcut;
    int flags = 0;
    int line = -1;
};
bool operator==(const MenuItem &left, const MenuItem &right);

struct Menu
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QVector<RcFile::MenuItem> children MEMBER children)
public:
    QString id;
    QVector<MenuItem> children;
    // Internal data
    int line = -1;
};

struct Shortcut
{
    Q_GADGET
    Q_PROPERTY(QString event MEMBER event)
    Q_PROPERTY(bool unknown MEMBER unknown)
public:
    QString event;
    bool unknown = false;
};
bool operator==(const Shortcut &left, const Shortcut &right);

struct Action
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString title MEMBER title)
    Q_PROPERTY(QVector<RcFile::Shortcut> shortcuts MEMBER shortcuts)
    Q_PROPERTY(QString toolTip MEMBER toolTip)
    Q_PROPERTY(QString statusTip MEMBER statusTip)
    Q_PROPERTY(QString iconPath MEMBER iconPath)
    Q_PROPERTY(bool checked MEMBER checked)
public:
    QString id;
    QString title;
    QVector<Shortcut> shortcuts;
    QString toolTip;
    QString statusTip;
    QString iconPath;
    bool checked = false;
};

//=============================================================================
// Data describing a full file
//=============================================================================
struct Data
{
    struct Include
    {
        int line = -1;
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

        bool isUnknown() const { return shortcut.contains("VK_"); }
    };
    struct AcceleratorTable
    {
        int line = -1;
        QString id;
        QVector<Accelerator> accelerators;
    };
    struct Control
    {
        int line = -1;
        int type;
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
    QVector<Asset> icons;
    QVector<Asset> assets;
    QHash<QString, String> strings;
    QVector<AcceleratorTable> acceleratorTables;
    QVector<Menu> menus;
    QVector<ToolBar> toolBars;
    QVector<DialogData> dialogData;
    QVector<Dialog> dialogs;

    QHash<int, QString> resourceMap;

    // Accessors
    const Asset *asset(const QString &id) const;
    const ToolBar *toolBar(const QString &id) const;
    const Dialog *dialog(const QString &id) const;
    const Menu *menu(const QString &id) const;
    const AcceleratorTable *acceleratorTable(const QString &id) const;
};

} // namespace RcFile

Q_DECLARE_METATYPE(RcFile::Asset)
Q_DECLARE_OPERATORS_FOR_FLAGS(RcFile::Asset::ConversionFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(RcFile::Asset::TransparentColors)
Q_DECLARE_METATYPE(QVector<RcFile::Asset>)
Q_DECLARE_METATYPE(RcFile::ToolBarItem)
Q_DECLARE_METATYPE(QVector<RcFile::ToolBarItem>)
Q_DECLARE_METATYPE(RcFile::ToolBar)
Q_DECLARE_METATYPE(QVector<RcFile::ToolBar>)
Q_DECLARE_METATYPE(RcFile::Widget)
Q_DECLARE_METATYPE(QVector<RcFile::Widget>)
Q_DECLARE_OPERATORS_FOR_FLAGS(RcFile::Widget::ConversionFlags)
Q_DECLARE_METATYPE(RcFile::MenuItem)
Q_DECLARE_METATYPE(QVector<RcFile::MenuItem>)
Q_DECLARE_METATYPE(RcFile::Menu)
Q_DECLARE_METATYPE(QVector<RcFile::Menu>)
Q_DECLARE_METATYPE(RcFile::Shortcut)
Q_DECLARE_METATYPE(QVector<RcFile::Shortcut>)
Q_DECLARE_METATYPE(RcFile::Action)
Q_DECLARE_METATYPE(QVector<RcFile::Action>)
