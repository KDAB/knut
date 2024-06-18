/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QVariant>
#include <QVector>

namespace RcCore {

struct RibbonElement
{
    Q_GADGET
    Q_PROPERTY(QString type MEMBER type)
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString text MEMBER text)
    Q_PROPERTY(QString keys MEMBER keys)
    Q_PROPERTY(int smallIndex MEMBER smallIndex)
    Q_PROPERTY(int largeIndex MEMBER largeIndex)
    Q_PROPERTY(QVector<RcCore::RibbonElement> elements MEMBER elements)

public:
    QString type;
    QString id;
    QString text;
    QString keys;
    int smallIndex = -1;
    int largeIndex = -1;
    QVector<RibbonElement> elements;

    bool isSeparator() const { return type == "Separator"; }
};
bool operator==(const RibbonElement &lhs, const RibbonElement &rhs);

struct RibbonPanel
{
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString keys MEMBER keys)
    Q_PROPERTY(QVector<RcCore::RibbonElement> elements MEMBER elements)

public:
    QString name;
    QString keys;
    QVector<RibbonElement> elements;

    bool operator==(const RibbonPanel &other) const = default;
};

struct RibbonCategory
{
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString keys MEMBER keys)
    Q_PROPERTY(QString smallImage MEMBER smallImage)
    Q_PROPERTY(QString largeImage MEMBER largeImage)
    Q_PROPERTY(QVector<RcCore::RibbonPanel> panels MEMBER panels)

public:
    QString name;
    QString keys;
    QString smallImage;
    QString largeImage;
    QVector<RibbonPanel> panels;

    bool operator==(const RibbonCategory &other) const = default;
};

struct RibbonContext
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString text MEMBER text)
    Q_PROPERTY(QVector<RcCore::RibbonCategory> categories MEMBER categories)

public:
    QString id;
    QString text;
    QVector<RibbonCategory> categories;

    bool operator==(const RibbonContext &other) const = default;
};

struct RibbonMenu
{
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString smallImage MEMBER smallImage)
    Q_PROPERTY(QString largeImage MEMBER largeImage)
    Q_PROPERTY(QVector<RcCore::RibbonElement> elements MEMBER elements)
    Q_PROPERTY(bool recentFiles MEMBER recentFiles)
public:
    QString name;
    QString smallImage;
    QString largeImage;
    QVector<RibbonElement> elements;
    bool recentFiles = false;

    bool operator==(const RibbonMenu &other) const = default;
};

struct Ribbon
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(RcCore::RibbonMenu menu MEMBER menu)
    Q_PROPERTY(QVector<RcCore::RibbonCategory> categories MEMBER categories)
    Q_PROPERTY(QVector<RcCore::RibbonContext> contexts MEMBER contexts)

public:
    QString id;
    RibbonMenu menu;
    QVector<RibbonCategory> categories;
    QVector<RibbonContext> contexts;
    // Internal data
    int line = -1;
    QString fileName;

    bool load();
};

} // namespace RcCore

Q_DECLARE_METATYPE(RcCore::RibbonElement)
Q_DECLARE_METATYPE(QVector<RcCore::RibbonElement>)
Q_DECLARE_METATYPE(RcCore::RibbonPanel)
Q_DECLARE_METATYPE(QVector<RcCore::RibbonPanel>)
Q_DECLARE_METATYPE(RcCore::RibbonCategory)
Q_DECLARE_METATYPE(QVector<RcCore::RibbonCategory>)
Q_DECLARE_METATYPE(RcCore::RibbonContext)
Q_DECLARE_METATYPE(QVector<RcCore::RibbonContext>)
Q_DECLARE_METATYPE(RcCore::RibbonMenu)
Q_DECLARE_METATYPE(RcCore::Ribbon)
Q_DECLARE_METATYPE(QVector<RcCore::Ribbon>)
