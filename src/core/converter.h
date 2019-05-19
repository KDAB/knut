#ifndef CONVERTER_H
#define CONVERTER_H

#include "global.h"

#include <QVariant>

struct Data;

namespace Converter {

struct Shortcut
{
    Q_GADGET
    Q_PROPERTY(QString event MEMBER event)
    Q_PROPERTY(bool unknown MEMBER unknown)

public:
    QString event;
    bool unknown = false;
};

struct Action
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString title MEMBER title)
    Q_PROPERTY(QVariantList shortcuts MEMBER shortcuts)
    Q_PROPERTY(QString toolTip MEMBER toolTip)
    Q_PROPERTY(QString statusTip MEMBER statusTip)
    Q_PROPERTY(bool checked MEMBER checked)

public:
    QString id;
    QString title;
    QVariantList shortcuts;
    QString toolTip;
    QString statusTip;
    bool checked = false;
};

struct MenuItem
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString title MEMBER title)
    Q_PROPERTY(bool isSeparator MEMBER isSeparator)
    Q_PROPERTY(bool isAction MEMBER isAction)
    Q_PROPERTY(bool isTopLevel MEMBER isTopLevel)
    Q_PROPERTY(QVariantList children MEMBER children)
    Q_PROPERTY(QVariantList actions MEMBER actions)

public:
    QString id;
    QString title;
    bool isSeparator = false;
    bool isAction = false;
    bool isTopLevel = false;
    QVariantList children;
    // Actions are only available from topLevel menus
    QVariantList actions;
};

struct Menu
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QVariantList children MEMBER children)

public:
    QString id;
    QVariantList children;
};

struct ToolBarItem
{
    Q_GADGET
    Q_PROPERTY(bool isSeparator MEMBER isSeparator)
    Q_PROPERTY(QString id MEMBER id)
public:
    QString id;
    bool isSeparator = false;
};

struct ToolBar
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QVariantList children MEMBER children)
    Q_PROPERTY(QSize iconSize MEMBER iconSize)
public:
    QString id;
    QSize iconSize;
    QVariantList children;
};

struct Widget
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString className MEMBER className)
    Q_PROPERTY(QRect geometry MEMBER geometry)
    Q_PROPERTY(QVariantMap properties MEMBER properties)
    Q_PROPERTY(QVariantList children MEMBER children)
public:
    QString id;
    QString className;
    QRect geometry;
    QVariantMap properties;
    QVariantList children;
};

QVariantList convertActions(Data *data, const Knut::DataCollection &collection);
Menu convertMenu(Data *data, const Knut::DataCollection &collection);
ToolBar convertToolbar(Data *data, const Knut::DataCollection &collection);

Widget convertDialog(Data *data, int index);
QVariantList convertDialogs(Data *data, const Knut::DataCollection &collection);
}

Q_DECLARE_METATYPE(Converter::Shortcut)
Q_DECLARE_METATYPE(Converter::Action)
Q_DECLARE_METATYPE(Converter::MenuItem)
Q_DECLARE_METATYPE(Converter::Menu)
Q_DECLARE_METATYPE(Converter::ToolBar)
Q_DECLARE_METATYPE(Converter::Widget)

#endif // CONVERTER_H
