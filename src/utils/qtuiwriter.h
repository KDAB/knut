/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QHash>
#include <QString>
#include <pugixml.hpp>

namespace Utils {

class QtUiWriter
{
public:
    QtUiWriter(pugi::xml_document &document);

    pugi::xml_node addWidget(const QString &className, const QString &name, pugi::xml_node parent = {});

    enum Status { Success, AlreadyExists, InvalidHeader, InvalidProperty };
    Status addCustomWidget(const QString &className, const QString &baseClassName, const QString &header,
                           bool isContainer = false);

    Status addWidgetProperty(pugi::xml_node widget, const QString &name, const QVariant &value,
                             const QHash<QString, QString> &attributes = {}, bool userProperty = true);

    Status setWidgetName(pugi::xml_node widget, const QString &name, bool isRoot = false);
    Status setWidgetClassName(pugi::xml_node widget, const QString &className);

    QString dump() const;

private:
    void initializeXml();

    pugi::xml_document &m_document;
};

} // namespace Utils
