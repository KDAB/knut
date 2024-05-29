/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QHash>
#include <QObject>
#include <QStack>
#include <QVector>
#include <QXmlStreamWriter>

namespace RcCore {

struct Widget;

class UiWriter : public QObject
{
    Q_OBJECT

public:
    UiWriter(QIODevice *device, QObject *parent = nullptr);
    ~UiWriter() override;

public slots:
    void setClassName(const QString &className);

    void addCustomWidget(const QString &className, const QString &baseClassName, const QString &header,
                         bool isContainer = false);

    void startWidget(const QString &className, const RcCore::Widget &widget);
    void endWidget();

    void addProperty(const QString &name, const QVariant &value);

private:
    QXmlStreamWriter m_writer;

    struct CustomWidget
    {
        QString className;
        QString baseClassName;
        QString header;
        bool isGlobal = false;
        bool isContainer = false;
    };
    QHash<QString, CustomWidget> m_customWidgets;
    QHash<QString, int> m_widgetName;

    QStack<bool> m_isMainWindow;
    QString m_currentId;
};

} // namespace RcCore
