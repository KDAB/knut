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

struct Menu
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString title MEMBER title)
    Q_PROPERTY(bool isSeparator MEMBER isSeparator)
    Q_PROPERTY(bool isAction MEMBER isAction)
    Q_PROPERTY(QVariantList children MEMBER children)

public:
    QString id;
    QString title;
    bool isSeparator = false;
    bool isAction = false;
    QVariantList children;
};

QVariantList convertActions(Data *data, const Knut::DataCollection &collection);
Menu convertMenu(Data *data, const Knut::DataCollection &collection);

}

Q_DECLARE_METATYPE(Converter::Shortcut)
Q_DECLARE_METATYPE(Converter::Action)
Q_DECLARE_METATYPE(Converter::Menu)

#endif // CONVERTER_H
