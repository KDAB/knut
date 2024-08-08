/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qtuiwriter.h"

#include <QRect>
#include <QVariant>
#include <sstream>

namespace Utils {

QtUiWriter::QtUiWriter(pugi::xml_document &document)
    : m_document(document)
{
    initializeXml();
}

pugi::xml_node QtUiWriter::addWidget(const QString &className, const QString &name, pugi::xml_node parent)
{
    auto uiNode = m_document.child("ui");
    const bool isRoot = parent.empty();

    // Special case of QMainWindow, which has an intermediary <widget> tag
    if (!isRoot && strcmp(parent.attribute("class").value(), "QMainWindow") == 0) {
        parent = parent.child("widget");
    }

    pugi::xml_node widget =
        isRoot ? uiNode.insert_child_after("widget", uiNode.child("class")) : parent.append_child("widget");
    widget.append_attribute("class").set_value(className.toLatin1().constData());
    widget.append_attribute("name").set_value(name.toLatin1().constData());

    if (isRoot) {
        // Also change the <class> tag
        auto node = widget.previous_sibling();
        node.text().set(name.toLatin1().constData());
    }

    if (className == "QMainWindow") {
        auto centralWidget = widget.append_child("widget");
        centralWidget.append_attribute("class").set_value("QWidget");
        centralWidget.append_attribute("name").set_value("centralwidget");
    }

    return widget;
}

QtUiWriter::Status QtUiWriter::addCustomWidget(const QString &className, const QString &baseClassName,
                                               const QString &header, bool isContainer)
{
    // Check if the custom widget already exists
    const QString customPath = "customwidget[@class='" % className % "']";
    auto customNode = m_document.select_node(customPath.toLatin1().constData()).node();
    if (!customNode.empty())
        return AlreadyExists;

    // Check if the header is valid
    if ((!header.startsWith('<') || !header.endsWith('>')) && (!header.startsWith('"') || !header.endsWith('"')))
        return InvalidHeader;

    const bool isGlobal = header.startsWith('<');
    const auto &include = header.mid(1, header.length() - 2);

    auto customWidgets = m_document.select_node("ui/customwidgets").node();
    if (customWidgets.empty())
        customWidgets = m_document.child("ui").append_child("customwidgets");

    customNode = customWidgets.append_child("customwidget");
    customNode.append_child("class").text().set(className.toLatin1().constData());
    customNode.append_child("extends").text().set(baseClassName.toLatin1().constData());
    auto headerNode = customNode.append_child("header");
    headerNode.text().set(include.toLatin1().constData());
    if (isGlobal)
        headerNode.append_attribute("location").set_value("global");
    if (isContainer)
        customNode.append_child("container").text().set("1");
    return Success;
}

QtUiWriter::Status QtUiWriter::addWidgetProperty(pugi::xml_node widget, const QString &name, const QVariant &value,
                                                 const QHash<QString, QString> &attributes, bool userProperty)
{
    // Special case for stringlist
    if (static_cast<QMetaType::Type>(value.typeId()) == QMetaType::QStringList) {
        const auto values = value.toStringList();
        for (const auto &text : values) {
            auto valueNode = widget.append_child("item").append_child("property");
            valueNode.append_attribute("name").set_value(name.toLatin1().constData());
            valueNode.append_child("string").text().set(text.toLatin1().constData());
        }
        return Success;
    }

    auto createPropertyNode = [&widget, name, userProperty]() {
        auto propertyNode = widget.append_child("property");
        propertyNode.append_attribute("name").set_value(name.toLatin1().constData());
        if (userProperty) {
            propertyNode.append_attribute("stdset").set_value("0");
        }
        return propertyNode;
    };

    pugi::xml_node dataNode;
    switch (static_cast<QMetaType::Type>(value.typeId())) {
    case QMetaType::QRect: {
        auto propertyNode = createPropertyNode();
        dataNode = propertyNode.append_child("rect");
        dataNode.append_child("x").text().set(value.toRect().x());
        dataNode.append_child("y").text().set(value.toRect().y());
        dataNode.append_child("width").text().set(value.toRect().width());
        dataNode.append_child("height").text().set(value.toRect().height());
        break;
    }
    case QMetaType::Bool: {
        auto propertyNode = createPropertyNode();
        dataNode = propertyNode.append_child("bool");
        dataNode.text().set(value.toBool());
        break;
    }
    case QMetaType::Int: {
        auto propertyNode = createPropertyNode();
        dataNode = propertyNode.append_child("number");
        dataNode.text().set(value.toInt());
        break;
    }
    case QMetaType::QString: {
        auto propertyNode = createPropertyNode();
        const auto text = value.toString();
        if (name.compare("alignment") == 0) {
            dataNode = propertyNode.append_child("set");
        } else {
            // Good enough for now, may need update for corner cases
            if (text.contains("::") && !text.contains(' '))
                dataNode = propertyNode.append_child("enum");
            else
                dataNode = propertyNode.append_child("string");
        }
        dataNode.text().set(text.toLatin1().constData());
        break;
    }
    default:
        return InvalidProperty;
    }

    for (const auto &attribute : attributes.asKeyValueRange()) {
        dataNode.append_attribute(attribute.first.toLatin1().constData())
            .set_value(attribute.second.toLatin1().constData());
    }
    return Success;
}

QtUiWriter::Status QtUiWriter::setWidgetName(pugi::xml_node widget, const QString &name, bool isRoot)
{
    widget.attribute("name").set_value(name.toLatin1().constData());

    widget.parent().child("class").text().set(name.toLatin1().constData());
    if (isRoot) {
        // Also change the <class> tag
        auto node = widget.previous_sibling();
        node.text().set(name.toLatin1().constData());
    }

    return Success;
}

QtUiWriter::Status QtUiWriter::setWidgetClassName(pugi::xml_node widget, const QString &className)
{
    widget.attribute("class").set_value(className.toLatin1().constData());
    return Success;
}

QString QtUiWriter::dump() const
{
    std::ostringstream ss;
    m_document.save(ss, "    ");
    return QString::fromStdString(ss.str());
}

// Create the basic skeleton of a Qt Designer UI file
void Utils::QtUiWriter::initializeXml()
{
    auto ui = m_document.select_node("ui");
    if (ui.node().empty()) {
        pugi::xml_node uiNode = m_document.append_child("ui");
        uiNode.append_attribute("version").set_value("4.0");
        uiNode.append_child("class");
        uiNode.append_child("resources");
        uiNode.append_child("connections");
    }
}

} // namespace Utils
