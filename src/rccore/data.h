/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "ribbon.h"

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QRect>
#include <QString>
#include <QVariant>

namespace RcCore {

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
    Q_FLAG(ConversionFlags)

    enum TransparentColor {
        NoColors = 0x08,
        Gray = 0x10,
        Magenta = 0x20,
        BottomLeftPixel = 0x40,
        AllColors = Gray | Magenta | BottomLeftPixel,
    };
    Q_DECLARE_FLAGS(TransparentColors, TransparentColor)
    Q_FLAG(TransparentColors)

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
    Q_PROPERTY(QList<RcCore::ToolBarItem> children MEMBER children)
    Q_PROPERTY(QSize iconSize MEMBER iconSize)
    Q_PROPERTY(QStringList actionIds READ actionIds)
public:
    QString id;
    QSize iconSize;
    QList<ToolBarItem> children;
    // Internal data
    int line = -1;

    Q_INVOKABLE bool contains(const QString &id) const;
    QStringList actionIds() const;
};

struct Widget
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString className MEMBER className)
    Q_PROPERTY(QRect geometry MEMBER geometry)
    Q_PROPERTY(QVariantMap properties MEMBER properties)
    Q_PROPERTY(QList<RcCore::Widget> children MEMBER children)
public:
    enum ConversionFlag {
        NoFlags = 0x00,
        UpdateHierarchy = 0x100,
        UpdateGeometry = 0x200,
        UseIdForPixmap = 0x400,
        AllFlags = UpdateHierarchy | UpdateGeometry | UseIdForPixmap,
    };
    Q_DECLARE_FLAGS(ConversionFlags, ConversionFlag)
    Q_FLAG(ConversionFlags)

    QString id;
    QString className;
    QRect geometry;
    QVariantMap properties;
    QList<Widget> children;
};
bool operator==(const Widget &left, const Widget &right);

struct MenuItem
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString text MEMBER text)
    Q_PROPERTY(QList<RcCore::MenuItem> children MEMBER children)
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
    QList<MenuItem> children;
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
    Q_PROPERTY(QList<RcCore::MenuItem> children MEMBER children)
    Q_PROPERTY(QStringList actionIds READ actionIds)
public:
    QString id;
    QList<MenuItem> children;
    // Internal data
    int line = -1;

    Q_INVOKABLE bool contains(const QString &id) const;
    QStringList actionIds() const;
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
    Q_PROPERTY(QList<RcCore::Shortcut> shortcuts MEMBER shortcuts)
    Q_PROPERTY(QString toolTip MEMBER toolTip)
    Q_PROPERTY(QString statusTip MEMBER statusTip)
    Q_PROPERTY(QString iconPath MEMBER iconPath)
    Q_PROPERTY(QString iconId MEMBER iconId)
    Q_PROPERTY(bool checked MEMBER checked)
public:
    QString id;
    QString title;
    QList<Shortcut> shortcuts;
    QString toolTip;
    QString statusTip;
    QString iconPath;
    QString iconId;
    bool checked = false;
};

using ActionList = QList<RcCore::Action>;

struct String
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString text MEMBER text)
public:
    QString id;
    QString text;
    // Internal data
    int line = -1;
};
bool operator==(const String &left, const String &right);

//=============================================================================
// Structure describing RC data for a given language
//=============================================================================
struct Data
{
    struct Include
    {
        int line = -1;
        QString fileName;
        bool exist = false;
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
        QList<Accelerator> accelerators;
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
        QList<Control> controls;
    };

    QString fileName;
    QString language;

    // Resources
    QList<Asset> icons;
    QList<Asset> assets;
    QHash<QString, String> strings;
    QList<AcceleratorTable> acceleratorTables;
    QList<Menu> menus;
    QList<ToolBar> toolBars;
    QList<DialogData> dialogDataList;
    QList<Dialog> dialogs;
    QList<Ribbon> ribbons;

    // Accessors
    const Asset *asset(const QString &id) const;
    const ToolBar *toolBar(const QString &id) const;
    const Dialog *dialog(const QString &id) const;
    const DialogData *dialogData(const QString &id) const;
    const Menu *menu(const QString &id) const;
    const AcceleratorTable *acceleratorTable(const QString &id) const;
    const Ribbon *ribbon(const QString &id) const;
};

} // namespace RcCore

Q_DECLARE_METATYPE(RcCore::Asset)
Q_DECLARE_OPERATORS_FOR_FLAGS(RcCore::Asset::ConversionFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(RcCore::Asset::TransparentColors)
Q_DECLARE_METATYPE(QList<RcCore::Asset>)
Q_DECLARE_METATYPE(RcCore::ToolBarItem)
Q_DECLARE_METATYPE(QList<RcCore::ToolBarItem>)
Q_DECLARE_METATYPE(RcCore::ToolBar)
Q_DECLARE_METATYPE(QList<RcCore::ToolBar>)
Q_DECLARE_METATYPE(RcCore::Widget)
Q_DECLARE_METATYPE(QList<RcCore::Widget>)
Q_DECLARE_OPERATORS_FOR_FLAGS(RcCore::Widget::ConversionFlags)
Q_DECLARE_METATYPE(RcCore::MenuItem)
Q_DECLARE_METATYPE(QList<RcCore::MenuItem>)
Q_DECLARE_METATYPE(RcCore::Menu)
Q_DECLARE_METATYPE(QList<RcCore::Menu>)
Q_DECLARE_METATYPE(RcCore::Shortcut)
Q_DECLARE_METATYPE(QList<RcCore::Shortcut>)
Q_DECLARE_METATYPE(RcCore::Action)
Q_DECLARE_METATYPE(QList<RcCore::Action>)
Q_DECLARE_METATYPE(RcCore::String)
Q_DECLARE_METATYPE(QList<RcCore::String>)
