/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QList>
#include <QVariant>

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
    Q_PROPERTY(QList<RcCore::RibbonElement> elements MEMBER elements)
    Q_PROPERTY(bool isSeparator READ isSeparator)

public:
    QString type;
    QString id;
    QString text;
    QString keys;
    int smallIndex = -1;
    int largeIndex = -1;
    QList<RibbonElement> elements;

    bool isSeparator() const { return type == "Separator"; }
};
bool operator==(const RibbonElement &lhs, const RibbonElement &rhs);

struct RibbonPanel
{
    Q_GADGET
    Q_PROPERTY(QString text MEMBER text)
    Q_PROPERTY(QString keys MEMBER keys)
    Q_PROPERTY(QList<RcCore::RibbonElement> elements MEMBER elements)

public:
    QString text;
    QString keys;
    QList<RibbonElement> elements;

    bool operator==(const RibbonPanel &other) const = default;
};

struct RibbonCategory
{
    Q_GADGET
    Q_PROPERTY(QString text MEMBER text)
    Q_PROPERTY(QString keys MEMBER keys)
    Q_PROPERTY(QString smallImage MEMBER smallImage)
    Q_PROPERTY(QString largeImage MEMBER largeImage)
    Q_PROPERTY(QList<RcCore::RibbonPanel> panels MEMBER panels)

public:
    QString text;
    QString keys;
    QString smallImage;
    QString largeImage;
    QList<RibbonPanel> panels;

    bool operator==(const RibbonCategory &other) const = default;
};

struct RibbonContext
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString text MEMBER text)
    Q_PROPERTY(QList<RcCore::RibbonCategory> categories MEMBER categories)

public:
    QString id;
    QString text;
    QList<RibbonCategory> categories;

    bool operator==(const RibbonContext &other) const = default;
};

struct RibbonMenu
{
    Q_GADGET
    Q_PROPERTY(QString text MEMBER text)
    Q_PROPERTY(QString smallImage MEMBER smallImage)
    Q_PROPERTY(QString largeImage MEMBER largeImage)
    Q_PROPERTY(QList<RcCore::RibbonElement> elements MEMBER elements)
    Q_PROPERTY(QString recentFilesText MEMBER recentFilesText)
public:
    QString text;
    QString smallImage;
    QString largeImage;
    QList<RibbonElement> elements;
    QString recentFilesText;

    bool operator==(const RibbonMenu &other) const = default;
};

struct Ribbon
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(RcCore::RibbonMenu menu MEMBER menu)
    Q_PROPERTY(QList<RcCore::RibbonCategory> categories MEMBER categories)
    Q_PROPERTY(QList<RcCore::RibbonContext> contexts MEMBER contexts)

public:
    QString id;
    RibbonMenu menu;
    QList<RibbonCategory> categories;
    QList<RibbonContext> contexts;
    // Internal data
    int line = -1;
    QString fileName;

    bool load();
    Q_INVOKABLE RcCore::RibbonElement elementFromId(const QString &id) const;
};

} // namespace RcCore

Q_DECLARE_METATYPE(RcCore::RibbonElement)
Q_DECLARE_METATYPE(QList<RcCore::RibbonElement>)
Q_DECLARE_METATYPE(RcCore::RibbonPanel)
Q_DECLARE_METATYPE(QList<RcCore::RibbonPanel>)
Q_DECLARE_METATYPE(RcCore::RibbonCategory)
Q_DECLARE_METATYPE(QList<RcCore::RibbonCategory>)
Q_DECLARE_METATYPE(RcCore::RibbonContext)
Q_DECLARE_METATYPE(QList<RcCore::RibbonContext>)
Q_DECLARE_METATYPE(RcCore::RibbonMenu)
Q_DECLARE_METATYPE(RcCore::Ribbon)
Q_DECLARE_METATYPE(QList<RcCore::Ribbon>)
